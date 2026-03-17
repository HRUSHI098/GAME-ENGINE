#include "GE/Physics/Physics2D.h"
#include "GE/Core/Log.h"

#include <box2d/box2d.h>

namespace GE {

// ─── ContactListener bridge ───────────────────────────────────────────────────
struct Physics2D::ContactListener : public b2ContactListener {
    Physics2D::ContactCallback OnBegin;
    Physics2D::ContactCallback OnEnd;

    void BeginContact(b2Contact* contact) override {
        if (!OnBegin) return;
        uintptr_t dataA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        uintptr_t dataB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        OnBegin({ static_cast<u32>(dataA), static_cast<u32>(dataB) });
    }

    void EndContact(b2Contact* contact) override {
        if (!OnEnd) return;
        uintptr_t dataA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        uintptr_t dataB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        OnEnd({ static_cast<u32>(dataA), static_cast<u32>(dataB) });
    }
};

// ─── Physics2D ────────────────────────────────────────────────────────────────
Physics2D::Physics2D() {
    b2Vec2 gravity(0.0f, -9.81f);
    m_World = new b2World(gravity);

    m_ContactListener = new ContactListener();
    m_World->SetContactListener(m_ContactListener);

    GE_CORE_INFO("Physics2D world created (gravity = 0, -9.81).");
}

Physics2D::~Physics2D() {
    delete m_World;
    delete m_ContactListener;
    m_World           = nullptr;
    m_ContactListener = nullptr;
}

void Physics2D::SetGravity(const glm::vec2& gravity) {
    m_World->SetGravity({ gravity.x, gravity.y });
}

glm::vec2 Physics2D::GetGravity() const {
    auto g = m_World->GetGravity();
    return { g.x, g.y };
}

void Physics2D::Step(f32 deltaTime, i32 velocityIterations, i32 positionIterations) {
    m_World->Step(deltaTime, velocityIterations, positionIterations);

    // Update callbacks are called from within Step via the ContactListener
    m_ContactListener->OnBegin = m_BeginContact;
    m_ContactListener->OnEnd   = m_EndContact;
}

} // namespace GE
