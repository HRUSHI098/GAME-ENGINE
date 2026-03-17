#pragma once

#include "GE/Core/Base.h"
#include "GE/Audio/AudioClip.h"
#include <string>

namespace GE {

// ─── AudioEngine — static API, SDL2_mixer backend ─────────────────────────────
//
// Sound effects (AudioClip) use Mix_Channels — up to 32 simultaneous.
// Music (MusicTrack)        uses Mix_Music   — single streaming track.
//
// All volume values are 0.0 … 1.0.
class AudioEngine {
public:
    static void Init();
    static void Shutdown();

    // ── Sound effects ────────────────────────────────────────────────────────
    // Returns the channel it was assigned to (-1 on failure).
    static i32  PlaySound(const Ref<AudioClip>& clip,
                          f32  volume = 1.0f,
                          bool loop   = false);

    static void StopSound(i32 channel);   // -1 = stop all channels
    static void PauseSound(i32 channel);
    static void ResumeSound(i32 channel);
    static bool IsSoundPlaying(i32 channel);

    // ── Music ─────────────────────────────────────────────────────────────────
    static void PlayMusic(const Ref<MusicTrack>& track,
                          f32  volume = 1.0f,
                          bool loop   = true);

    static void StopMusic();
    static void PauseMusic();
    static void ResumeMusic();
    static bool IsMusicPlaying();
    static bool IsMusicPaused();
    static void SetMusicVolume(f32 volume);   // 0..1

    // ── Global volumes ────────────────────────────────────────────────────────
    static void SetMasterSoundVolume(f32 volume);   // applies to all channels
    static f32  GetMasterSoundVolume();
    static f32  GetMusicVolume();
};

} // namespace GE
