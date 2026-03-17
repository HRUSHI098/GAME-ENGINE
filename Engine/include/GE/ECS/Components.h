#pragma once

#include "GE/Core/Base.h"
#include "GE/Renderer/Texture.h"
#include "GE/Renderer/Camera.h"
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <functional>

namespace GE {

// ─── TagComponent ─────────────────────────────────────────────────────────────
struct TagComponent {
    std::string Tag;
    TagComponent() = default;
    explicit TagComponent(std::string tag) : Tag(std::move(tag)) {}
};

// ─── TransformComponent ───────────────────────────────────────────────────────
struct TransformComponent {
    glm::vec2 Position = { 0.0f, 0.0f };
    glm::vec2 Scale    = { 1.0f, 1.0f };
    f32       Rotation = 0.0f;          // radians

    TransformComponent() = default;
    TransformComponent(glm::vec2 pos, glm::vec2 scale = { 1.0f, 1.0f }, f32 rot = 0.0f)
        : Position(pos), Scale(scale), Rotation(rot) {}
};

// ─── SpriteRendererComponent ──────────────────────────────────────────────────
struct SpriteRendererComponent {
    glm::vec4      Color   = { 1.0f, 1.0f, 1.0f, 1.0f };
    Ref<Texture2D> Texture = nullptr;
    // Sprite sheet / UV support will be added in the texture atlas phase
    i32 ZOrder = 0;    // draw order within the scene

    SpriteRendererComponent() = default;
    explicit SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
    explicit SpriteRendererComponent(Ref<Texture2D> tex, const glm::vec4& tint = glm::vec4(1.0f))
        : Color(tint), Texture(std::move(tex)) {}
};

// ─── CameraComponent ─────────────────────────────────────────────────────────
struct CameraComponent {
    Camera2D  Camera;
    bool      Primary          = true;
    bool      FixedAspectRatio = false;
    f32       OrthographicSize = 7.0f;          // half-height in world units
    glm::vec4 BackgroundColor  = { 0.1f, 0.1f, 0.1f, 1.0f };

    CameraComponent()
        : Camera(-7.0f * (16.f / 9.f), 7.0f * (16.f / 9.f), -7.0f, 7.0f) {}

    // Recalculate projection when OrthographicSize or aspect changes
    void RecalcProjection(float aspect) {
        Camera.SetProjection(
            -OrthographicSize * aspect,  OrthographicSize * aspect,
            -OrthographicSize,            OrthographicSize);
    }
};

// ─── RelationshipComponent ────────────────────────────────────────────────────
// Stores parent/child links for scene graph hierarchy.
struct RelationshipComponent {
    entt::entity Parent   = entt::null;
    std::vector<entt::entity> Children;
};

// ─── NativeScriptComponent ────────────────────────────────────────────────────
// Lightweight C++ scripting before the C# layer is ready.
// Usage:
//   entity.AddComponent<NativeScriptComponent>().Bind<PlayerController>();
class Entity;   // forward declared — full def in Entity.h

struct NativeScriptComponent {
    struct ScriptInstance {
        virtual ~ScriptInstance() = default;
        virtual void OnCreate()              {}
        virtual void OnDestroy()             {}
        virtual void OnUpdate(f32 deltaTime) {}
    };

    ScriptInstance* Instance = nullptr;

    std::function<ScriptInstance*()> InstantiateFn;
    std::function<void()>            DestroyFn;

    template<typename T>
    void Bind() {
        InstantiateFn = []() -> ScriptInstance* { return new T(); };
        DestroyFn     = [this]() { delete Instance; Instance = nullptr; };
    }
};

} // namespace GE
