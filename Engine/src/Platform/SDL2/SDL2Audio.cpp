#include "GE/Audio/AudioEngine.h"
#include "GE/Audio/AudioClip.h"
#include "GE/Core/Log.h"

#include <SDL2/SDL.h>
#include <SDL_mixer.h>
#include <algorithm>

// ─── MIX_MAX_VOLUME is 128; we normalise to 0..1 at the API surface ──────────
static constexpr int GE_MIX_CHANNELS = 32;

namespace GE {

// ══════════════════════════════════════════════════════════════════════════════
// AudioClip — SDL2 implementation
// ══════════════════════════════════════════════════════════════════════════════
class SDL2AudioClip : public AudioClip {
public:
    explicit SDL2AudioClip(const std::string& path) : m_Path(path) {
        m_Chunk = Mix_LoadWAV(path.c_str());
        if (!m_Chunk)
            GE_CORE_ERROR("Failed to load audio clip '{}': {}", path, Mix_GetError());
    }

    ~SDL2AudioClip() override {
        if (m_Chunk) { Mix_FreeChunk(m_Chunk); m_Chunk = nullptr; }
    }

    const std::string& GetPath()         const override { return m_Path; }
    f32                GetDuration()     const override { return 0.0f; }   // not exposed by SDL2_mixer
    void*              GetNativeHandle() const override { return m_Chunk; }

private:
    std::string m_Path;
    Mix_Chunk*  m_Chunk = nullptr;
};

Ref<AudioClip> AudioClip::Create(const std::string& path) {
    return MakeRef<SDL2AudioClip>(path);
}

// ══════════════════════════════════════════════════════════════════════════════
// MusicTrack — SDL2 implementation
// ══════════════════════════════════════════════════════════════════════════════
class SDL2MusicTrack : public MusicTrack {
public:
    explicit SDL2MusicTrack(const std::string& path) : m_Path(path) {
        m_Music = Mix_LoadMUS(path.c_str());
        if (!m_Music)
            GE_CORE_ERROR("Failed to load music '{}': {}", path, Mix_GetError());
    }

    ~SDL2MusicTrack() override {
        if (m_Music) { Mix_FreeMusic(m_Music); m_Music = nullptr; }
    }

    const std::string& GetPath()         const override { return m_Path; }
    void*              GetNativeHandle() const override { return m_Music; }

private:
    std::string m_Path;
    Mix_Music*  m_Music = nullptr;
};

Ref<MusicTrack> MusicTrack::Create(const std::string& path) {
    return MakeRef<SDL2MusicTrack>(path);
}

// ══════════════════════════════════════════════════════════════════════════════
// AudioEngine — static state + SDL2_mixer calls
// ══════════════════════════════════════════════════════════════════════════════
static f32 s_MasterSoundVolume = 1.0f;
static f32 s_MusicVolume       = 1.0f;

void AudioEngine::Init() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        GE_CORE_ERROR("SDL audio init failed: {}", SDL_GetError());
        return;
    }

    // 44100 Hz, stereo, 2048-sample buffer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        GE_CORE_ERROR("Mix_OpenAudio failed: {}", Mix_GetError());
        return;
    }

    Mix_AllocateChannels(GE_MIX_CHANNELS);
    GE_CORE_INFO("AudioEngine ready ({} channels, 44100 Hz).", GE_MIX_CHANNELS);
}

void AudioEngine::Shutdown() {
    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    GE_CORE_INFO("AudioEngine shut down.");
}

// ─── Sound effects ────────────────────────────────────────────────────────────
i32 AudioEngine::PlaySound(const Ref<AudioClip>& clip, f32 volume, bool loop) {
    if (!clip) return -1;
    auto* chunk = static_cast<Mix_Chunk*>(clip->GetNativeHandle());
    if (!chunk) return -1;

    int vol = static_cast<int>(std::clamp(volume * s_MasterSoundVolume, 0.0f, 1.0f) * MIX_MAX_VOLUME);
    Mix_VolumeChunk(chunk, vol);

    int channel = Mix_PlayChannel(-1, chunk, loop ? -1 : 0);
    if (channel == -1)
        GE_CORE_WARN("No free audio channel for '{}'.", clip->GetPath());
    return channel;
}

void AudioEngine::StopSound(i32 channel)   { Mix_HaltChannel(channel); }
void AudioEngine::PauseSound(i32 channel)  { Mix_Pause(channel); }
void AudioEngine::ResumeSound(i32 channel) { Mix_Resume(channel); }
bool AudioEngine::IsSoundPlaying(i32 channel) {
    return Mix_Playing(channel) != 0 && Mix_Paused(channel) == 0;
}

// ─── Music ────────────────────────────────────────────────────────────────────
void AudioEngine::PlayMusic(const Ref<MusicTrack>& track, f32 volume, bool loop) {
    if (!track) return;
    auto* music = static_cast<Mix_Music*>(track->GetNativeHandle());
    if (!music) return;

    SetMusicVolume(volume);
    if (Mix_PlayMusic(music, loop ? -1 : 0) == -1)
        GE_CORE_ERROR("Mix_PlayMusic failed: {}", Mix_GetError());
}

void AudioEngine::StopMusic()    { Mix_HaltMusic(); }
void AudioEngine::PauseMusic()   { Mix_PauseMusic(); }
void AudioEngine::ResumeMusic()  { Mix_ResumeMusic(); }
bool AudioEngine::IsMusicPlaying() { return Mix_PlayingMusic() && !Mix_PausedMusic(); }
bool AudioEngine::IsMusicPaused()  { return Mix_PausedMusic() != 0; }

void AudioEngine::SetMusicVolume(f32 volume) {
    s_MusicVolume = std::clamp(volume, 0.0f, 1.0f);
    Mix_VolumeMusic(static_cast<int>(s_MusicVolume * MIX_MAX_VOLUME));
}

// ─── Global volumes ───────────────────────────────────────────────────────────
void AudioEngine::SetMasterSoundVolume(f32 volume) {
    s_MasterSoundVolume = std::clamp(volume, 0.0f, 1.0f);
    int vol = static_cast<int>(s_MasterSoundVolume * MIX_MAX_VOLUME);
    Mix_Volume(-1, vol);   // apply to all channels
}

f32 AudioEngine::GetMasterSoundVolume() { return s_MasterSoundVolume; }
f32 AudioEngine::GetMusicVolume()       { return s_MusicVolume; }

} // namespace GE
