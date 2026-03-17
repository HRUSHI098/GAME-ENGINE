#include "GE/Scripting/ScriptEngine.h"
#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"
#include "GE/ECS/Components.h"
#include "GE/Physics/PhysicsComponents.h"
#include "GE/Core/Input.h"
#include "GE/Core/Log.h"

#include <mono/jit/jit.h>
#include <mono/metadata/object.h>

#include <box2d/box2d.h>
#include <glm/glm.hpp>

// ─── Helper: entity from ID ───────────────────────────────────────────────────
static GE::Entity GetEntity(uint32_t id) {
    GE::Scene* scene = GE::ScriptEngine::GetSceneContext();
    GE_ASSERT(scene, "No active scene context in ScriptGlue");
    return { static_cast<entt::entity>(id), scene };
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal call implementations — these are registered with Mono and called
// from C# scripts via [MethodImpl(MethodImplOptions.InternalCall)].
// ─────────────────────────────────────────────────────────────────────────────

// ── Log ───────────────────────────────────────────────────────────────────────
static void Log_Trace(MonoString* msg) {
    char* s = mono_string_to_utf8(msg); GE_TRACE("{}", s); mono_free(s);
}
static void Log_Info(MonoString* msg) {
    char* s = mono_string_to_utf8(msg); GE_INFO("{}", s); mono_free(s);
}
static void Log_Warn(MonoString* msg) {
    char* s = mono_string_to_utf8(msg); GE_WARN("{}", s); mono_free(s);
}
static void Log_Error(MonoString* msg) {
    char* s = mono_string_to_utf8(msg); GE_ERROR("{}", s); mono_free(s);
}

// ── Entity ────────────────────────────────────────────────────────────────────
static bool Entity_HasComponent(uint32_t id, MonoReflectionType* componentType) {
    // Map the managed type to a C++ check
    MonoType* monoType = mono_reflection_type_get_type(componentType);
    const char* typeName = mono_type_get_name(monoType);
    GE::Entity entity = GetEntity(id);

    if (strstr(typeName, "TransformComponent"))  return entity.HasComponent<GE::TransformComponent>();
    if (strstr(typeName, "RigidBody2DComponent")) return entity.HasComponent<GE::RigidBody2DComponent>();
    if (strstr(typeName, "SpriteRenderer"))       return entity.HasComponent<GE::SpriteRendererComponent>();
    return false;
}

static uint32_t Entity_FindByName(MonoString* nameStr) {
    char* name = mono_string_to_utf8(nameStr);
    GE::Entity found = GE::ScriptEngine::GetSceneContext()->FindByName(name);
    mono_free(name);
    return found ? static_cast<uint32_t>(found) : UINT32_MAX;
}

// ── TransformComponent ────────────────────────────────────────────────────────
static void Transform_GetPosition(uint32_t id, glm::vec2* out) {
    *out = GetEntity(id).GetComponent<GE::TransformComponent>().Position;
}
static void Transform_SetPosition(uint32_t id, glm::vec2* val) {
    GetEntity(id).GetComponent<GE::TransformComponent>().Position = *val;
}
static void Transform_GetScale(uint32_t id, glm::vec2* out) {
    *out = GetEntity(id).GetComponent<GE::TransformComponent>().Scale;
}
static void Transform_SetScale(uint32_t id, glm::vec2* val) {
    GetEntity(id).GetComponent<GE::TransformComponent>().Scale = *val;
}
static float Transform_GetRotation(uint32_t id) {
    return GetEntity(id).GetComponent<GE::TransformComponent>().Rotation;
}
static void Transform_SetRotation(uint32_t id, float rot) {
    GetEntity(id).GetComponent<GE::TransformComponent>().Rotation = rot;
}

// ── RigidBody2DComponent ──────────────────────────────────────────────────────
static void RigidBody2D_GetLinearVelocity(uint32_t id, glm::vec2* out) {
    auto& rb = GetEntity(id).GetComponent<GE::RigidBody2DComponent>();
    if (rb.RuntimeBody) {
        const b2Vec2& v = rb.RuntimeBody->GetLinearVelocity();
        *out = { v.x, v.y };
    } else {
        *out = { 0.0f, 0.0f };
    }
}
static void RigidBody2D_SetLinearVelocity(uint32_t id, glm::vec2* val) {
    auto& rb = GetEntity(id).GetComponent<GE::RigidBody2DComponent>();
    if (rb.RuntimeBody) rb.RuntimeBody->SetLinearVelocity({ val->x, val->y });
}
static void RigidBody2D_ApplyLinearImpulse(uint32_t id, glm::vec2* impulse, bool wake) {
    auto& rb = GetEntity(id).GetComponent<GE::RigidBody2DComponent>();
    if (rb.RuntimeBody)
        rb.RuntimeBody->ApplyLinearImpulseToCenter({ impulse->x, impulse->y }, wake);
}
static void RigidBody2D_ApplyForce(uint32_t id, glm::vec2* force, bool wake) {
    auto& rb = GetEntity(id).GetComponent<GE::RigidBody2DComponent>();
    if (rb.RuntimeBody)
        rb.RuntimeBody->ApplyForceToCenter({ force->x, force->y }, wake);
}

// ── Input ─────────────────────────────────────────────────────────────────────
static bool Input_IsKeyPressed(GE::KeyCode key) {
    return GE::Input::IsKeyPressed(key);
}
static bool Input_IsMouseButtonPressed(GE::MouseButton button) {
    return GE::Input::IsMouseButtonPressed(button);
}
static void Input_GetMousePosition(float* x, float* y) {
    auto [mx, my] = GE::Input::GetMousePosition();
    *x = mx; *y = my;
}

// ─── Registration ─────────────────────────────────────────────────────────────
namespace GE {

void ScriptEngine::RegisterInternalCalls() {
#define GE_ADD_INTERNAL_CALL(Name) \
    mono_add_internal_call("GE." #Name, reinterpret_cast<void*>(Name))

    GE_ADD_INTERNAL_CALL(Log_Trace);
    GE_ADD_INTERNAL_CALL(Log_Info);
    GE_ADD_INTERNAL_CALL(Log_Warn);
    GE_ADD_INTERNAL_CALL(Log_Error);

    GE_ADD_INTERNAL_CALL(Entity_HasComponent);
    GE_ADD_INTERNAL_CALL(Entity_FindByName);

    GE_ADD_INTERNAL_CALL(Transform_GetPosition);
    GE_ADD_INTERNAL_CALL(Transform_SetPosition);
    GE_ADD_INTERNAL_CALL(Transform_GetScale);
    GE_ADD_INTERNAL_CALL(Transform_SetScale);
    GE_ADD_INTERNAL_CALL(Transform_GetRotation);
    GE_ADD_INTERNAL_CALL(Transform_SetRotation);

    GE_ADD_INTERNAL_CALL(RigidBody2D_GetLinearVelocity);
    GE_ADD_INTERNAL_CALL(RigidBody2D_SetLinearVelocity);
    GE_ADD_INTERNAL_CALL(RigidBody2D_ApplyLinearImpulse);
    GE_ADD_INTERNAL_CALL(RigidBody2D_ApplyForce);

    GE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
    GE_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
    GE_ADD_INTERNAL_CALL(Input_GetMousePosition);

#undef GE_ADD_INTERNAL_CALL
}

} // namespace GE
