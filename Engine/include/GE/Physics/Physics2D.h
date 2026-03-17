#pragma once

#include "GE/Core/Base.h"
#include <glm/glm.hpp>
#include <functional>

class b2World;
class b2Contact;

namespace GE {

// ─── Collision contact info passed to callbacks ───────────────────────────────
struct ContactInfo {
    u32 EntityA;   // entt::entity cast to u32
    u32 EntityB;
};

// ─── Physics2D ────────────────────────────────────────────────────────────────
// Thin wrapper around a b2World. The Scene owns one of these.
class Physics2D {
public:
    using ContactCallback = std::function<void(const ContactInfo&)>;

    Physics2D();
    ~Physics2D();

    void SetGravity(const glm::vec2& gravity);
    glm::vec2 GetGravity() const;

    // Called once per frame. velocityIterations / positionIterations trade
    // accuracy vs. performance; Box2D recommends 8/3.
    void Step(f32 deltaTime,
              i32 velocityIterations = 8,
              i32 positionIterations = 3);

    void SetBeginContactCallback(ContactCallback cb) { m_BeginContact = std::move(cb); }
    void SetEndContactCallback(ContactCallback   cb) { m_EndContact   = std::move(cb); }

    b2World* GetWorld() { return m_World; }

private:
    b2World*        m_World        = nullptr;
    ContactCallback m_BeginContact;
    ContactCallback m_EndContact;

    // Inner contact listener bridges Box2D callbacks → our lambdas
    struct ContactListener;
    ContactListener* m_ContactListener = nullptr;
};

} // namespace GE
