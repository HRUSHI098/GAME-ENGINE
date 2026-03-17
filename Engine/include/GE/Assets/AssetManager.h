#pragma once
#include "GE/Core/Base.h"
#include "GE/Renderer/Texture.h"
#include "GE/Audio/AudioClip.h"
#include <string>
#include <unordered_map>

namespace GE {

// ─── AssetManager ─────────────────────────────────────────────────────────────
// Simple reference-counted cache. Assets are loaded once and reused.
// All paths are relative to the working directory.
class AssetManager {
public:
    static void Clear();   // release all cached assets

    // ── Textures ─────────────────────────────────────────────────────────────
    static Ref<Texture2D>  GetTexture(const std::string& path);

    // ── Audio ─────────────────────────────────────────────────────────────────
    static Ref<AudioClip>  GetSound(const std::string& path);
    static Ref<MusicTrack> GetMusic(const std::string& path);

    // ── Cache stats ───────────────────────────────────────────────────────────
    static size_t TextureCount() { return s_Textures.size(); }
    static size_t SoundCount()   { return s_Sounds.size();   }
    static size_t MusicCount()   { return s_Music.size();    }

private:
    static std::unordered_map<std::string, Ref<Texture2D>>  s_Textures;
    static std::unordered_map<std::string, Ref<AudioClip>>  s_Sounds;
    static std::unordered_map<std::string, Ref<MusicTrack>> s_Music;
};

} // namespace GE
