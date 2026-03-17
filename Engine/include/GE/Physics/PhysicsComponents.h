#pragma once

#include "GE/Core/Base.h"
#include <glm/glm.hpp>

// Forward-declare Box2D types so game code doesn't need to include Box2D headers
class b2Body;
class b2Fixture;

namespace GE {

// ─── RigidBody2DComponent ─────────────────────────────────────────────────────
struct RigidBody2DComponent {
    enum class BodyType { Static = 0, Dynamic, Kinematic };

    BodyType Type          = BodyType::Dynamic;
    bool     FixedRotation = false;

    // Runtime — set by the physics world on scene start
    b2Body* RuntimeBody = nullptr;
};

// ─── BoxCollider2DComponent ───────────────────────────────────────────────────
struct BoxCollider2DComponent {
    glm::vec2 Offset      = { 0.0f, 0.0f };
    glm::vec2 Size        = { 0.5f, 0.5f };   // half-extents in world units

    f32  Density          = 1.0f;
    f32  Friction         = 0.5f;
    f32  Restitution      = 0.0f;   // bounciness 0..1
    f32  RestitutionThreshold = 0.5f;

    // Runtime
    b2Fixture* RuntimeFixture = nullptr;
};

// ─── CircleCollider2DComponent ────────────────────────────────────────────────
struct CircleCollider2DComponent {
    glm::vec2 Offset = { 0.0f, 0.0f };
    f32       Radius = 0.5f;

    f32  Density              = 1.0f;
    f32  Friction             = 0.5f;
    f32  Restitution          = 0.0f;
    f32  RestitutionThreshold = 0.5f;

    // Runtime
    b2Fixture* RuntimeFixture = nullptr;
};

} // namespace GE
