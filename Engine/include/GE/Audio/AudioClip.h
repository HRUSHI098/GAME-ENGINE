#pragma once

#include "GE/Core/Base.h"
#include <string>

// Forward-declare SDL2_mixer types so game code stays clean
struct Mix_Chunk;
typedef struct _Mix_Music Mix_Music;

namespace GE {

// ─── AudioClip — short sound effect (loaded fully into memory) ────────────────
class AudioClip {
public:
    virtual ~AudioClip() = default;

    virtual const std::string& GetPath()     const = 0;
    virtual f32                GetDuration() const = 0;   // seconds, 0 if unknown
    virtual void*              GetNativeHandle() const = 0;

    static Ref<AudioClip> Create(const std::string& path);
};

// ─── MusicTrack — long-form audio streamed from disk ─────────────────────────
class MusicTrack {
public:
    virtual ~MusicTrack() = default;

    virtual const std::string& GetPath()        const = 0;
    virtual void*              GetNativeHandle() const = 0;

    static Ref<MusicTrack> Create(const std::string& path);
};

} // namespace GE
