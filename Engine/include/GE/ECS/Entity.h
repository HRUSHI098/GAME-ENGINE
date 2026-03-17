#pragma once

#include "GE/Core/Base.h"
#include "GE/Core/Log.h"
#include <entt/entt.hpp>

namespace GE {

class Scene;

// ─── Entity ───────────────────────────────────────────────────────────────────
// Thin handle — just an entt::entity + pointer back to the owning registry.
// Cheap to copy; the actual data lives in the Scene's registry.
class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene)
        : m_Handle(handle), m_Scene(scene) {}

    // ── Component API ────────────────────────────────────────────────────────
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        GE_ASSERT(!HasComponent<T>(), "Entity already has component!");
        return GetRegistry().emplace<T>(m_Handle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent() {
        GE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        return GetRegistry().get<T>(m_Handle);
    }

    template<typename T>
    const T& GetComponent() const {
        GE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        return GetRegistry().get<T>(m_Handle);
    }

    template<typename T>
    bool HasComponent() const {
        return GetRegistry().all_of<T>(m_Handle);
    }

    template<typename T>
    void RemoveComponent() {
        GE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        GetRegistry().remove<T>(m_Handle);
    }

    // ── Validity ──────────────────────────────────────────────────────────────
    bool   IsValid()   const { return m_Scene != nullptr && m_Handle != entt::null; }
    Scene* GetScene()  const { return m_Scene; }

    operator bool()           const { return IsValid(); }
    operator entt::entity()   const { return m_Handle; }
    operator u32()            const { return static_cast<u32>(m_Handle); }

    bool operator==(const Entity& other) const {
        return m_Handle == other.m_Handle && m_Scene == other.m_Scene;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::registry& GetRegistry() const;   // implemented in Scene.cpp

    entt::entity m_Handle = entt::null;
    Scene*       m_Scene  = nullptr;
};

} // namespace GE
