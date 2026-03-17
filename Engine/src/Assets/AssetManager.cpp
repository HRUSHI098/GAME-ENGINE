#include "GE/Assets/AssetManager.h"
#include "GE/Core/Log.h"

namespace GE {

std::unordered_map<std::string, Ref<Texture2D>>  AssetManager::s_Textures;
std::unordered_map<std::string, Ref<AudioClip>>  AssetManager::s_Sounds;
std::unordered_map<std::string, Ref<MusicTrack>> AssetManager::s_Music;

void AssetManager::Clear() {
    s_Textures.clear();
    s_Sounds.clear();
    s_Music.clear();
    GE_CORE_INFO("AssetManager: cache cleared.");
}

Ref<Texture2D> AssetManager::GetTexture(const std::string& path) {
    auto it = s_Textures.find(path);
    if (it != s_Textures.end()) return it->second;

    auto tex = Texture2D::Create(path);
    if (tex) {
        s_Textures[path] = tex;
        GE_CORE_TRACE("AssetManager: loaded texture '{}'", path);
    }
    return tex;
}

Ref<AudioClip> AssetManager::GetSound(const std::string& path) {
    auto it = s_Sounds.find(path);
    if (it != s_Sounds.end()) return it->second;

    auto clip = AudioClip::Create(path);
    if (clip) {
        s_Sounds[path] = clip;
        GE_CORE_TRACE("AssetManager: loaded sound '{}'", path);
    }
    return clip;
}

Ref<MusicTrack> AssetManager::GetMusic(const std::string& path) {
    auto it = s_Music.find(path);
    if (it != s_Music.end()) return it->second;

    auto track = MusicTrack::Create(path);
    if (track) {
        s_Music[path] = track;
        GE_CORE_TRACE("AssetManager: loaded music '{}'", path);
    }
    return track;
}

} // namespace GE
