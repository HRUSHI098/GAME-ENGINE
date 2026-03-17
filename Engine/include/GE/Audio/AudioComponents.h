#pragma once

#include "GE/Core/Base.h"
#include "GE/Audio/AudioClip.h"

namespace GE {

// ─── AudioSourceComponent ─────────────────────────────────────────────────────
// Attach to any entity to give it a sound.
// Scene::OnPhysicsStart calls AudioEngine::PlaySound for PlayOnAwake sources.
struct AudioSourceComponent {
    Ref<AudioClip> Clip        = nullptr;
    f32            Volume      = 1.0f;
    bool           Loop        = false;
    bool           PlayOnAwake = false;

    // Runtime — channel assigned by AudioEngine::PlaySound (-1 = not playing)
    i32 Channel = -1;
};

// ─── AudioListenerComponent ───────────────────────────────────────────────────
// Mark an entity as the audio listener (typically the camera entity).
// Spatial audio attenuation will be driven by this in a future pass.
struct AudioListenerComponent {
    bool Primary = true;
};

} // namespace GE
