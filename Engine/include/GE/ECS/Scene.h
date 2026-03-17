#pragma once

#include "GE/Core/Base.h"
#include "GE/Physics/Physics2D.h"
#include <entt/entt.hpp>
#include <string>

namespace GE {

class Entity;

// ─── Scene ────────────────────────────────────────────────────────────────────
// Owns the entt::registry and the Physics2D world.
// Drives the per-frame update → physics → render pipeline.
class Scene {
public:
    explicit Scene(std::string name = "Untitled");
    ~Scene();

    // ── Entity lifecycle ─────────────────────────────────────────────────────
    Entity CreateEntity(const std::string& name = "Entity");
    void   DestroyEntity(Entity entity);

    // ── Physics control ──────────────────────────────────────────────────────
    // Call once after populating the scene. Creates b2Bodies for all
    // RigidBody2DComponent / collider entities.
    void OnPhysicsStart();
    void OnPhysicsStop();

    void SetGravity(const glm::vec2& gravity);

    // Register collision callbacks (called from within OnUpdate's physics step)
    void SetBeginContactCallback(Physics2D::ContactCallback cb);
    void SetEndContactCallback  (Physics2D::ContactCallback cb);

    // ── Per-frame ────────────────────────────────────────────────────────────
    void OnUpdate(f32 deltaTime);
    void OnRender();
    void OnViewportResize(u32 width, u32 height);

    // ── Queries ──────────────────────────────────────────────────────────────
    template<typename... Components, typename Func>
    void View(Func&& func) {
        auto view = m_Registry.view<Components...>();
        view.each(std::forward<Func>(func));
    }

    Entity FindByName(const std::string& name);

    const std::string& GetName()          const { return m_Name; }
    entt::registry&    GetRegistry()            { return m_Registry; }
    u32                GetViewportWidth()  const { return m_ViewportWidth; }
    u32                GetViewportHeight() const { return m_ViewportHeight; }

private:
    entt::registry        m_Registry;
    std::string           m_Name;
    u32                   m_ViewportWidth  = 1280;
    u32                   m_ViewportHeight = 720;

    Scope<Physics2D>      m_Physics;   // null until OnPhysicsStart()
};

} // namespace GE
