// EngineBindings.cpp
// Registers the `ge_engine` Python module — importable from any AI script as:
//   import ge_engine as ge
//
// This file must be compiled into the engine (not a separate .pyd) since we
// use PYBIND11_EMBEDDED_MODULE which registers at link time.

#include "GE/Core/Base.h"
#include "GE/AI/PythonEngine.h"
#include "GE/AI/MLEnvironment.h"
#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"
#include "GE/ECS/Components.h"
#include "GE/Physics/PhysicsComponents.h"
#include "GE/Core/Input.h"
#include "GE/Core/Log.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <box2d/box2d.h>
#include <glm/glm.hpp>

namespace py = pybind11;

// ─────────────────────────────────────────────────────────────────────────────
PYBIND11_EMBEDDED_MODULE(ge_engine, m) {
    m.doc() = "Game Engine Python API";

    // ── Vector2 ───────────────────────────────────────────────────────────────
    py::class_<glm::vec2>(m, "Vector2")
        .def(py::init<float, float>(), py::arg("x") = 0.f, py::arg("y") = 0.f)
        .def_readwrite("x", &glm::vec2::x)
        .def_readwrite("y", &glm::vec2::y)
        .def("__repr__", [](const glm::vec2& v) {
            return "Vector2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        })
        .def("__add__", [](const glm::vec2& a, const glm::vec2& b){ return a + b; })
        .def("__sub__", [](const glm::vec2& a, const glm::vec2& b){ return a - b; })
        .def("__mul__", [](const glm::vec2& a, float s){ return a * s; })
        .def("length",  [](const glm::vec2& v){ return glm::length(v); });

    // ── Log ───────────────────────────────────────────────────────────────────
    py::class_<GE::Log>(m, "Log")
        .def_static("trace", [](const std::string& s){ GE_TRACE("{}", s); })
        .def_static("info",  [](const std::string& s){ GE_INFO("{}", s); })
        .def_static("warn",  [](const std::string& s){ GE_WARN("{}", s); })
        .def_static("error", [](const std::string& s){ GE_ERROR("{}", s); });

    // ── Input ─────────────────────────────────────────────────────────────────
    m.def("is_key_pressed", [](int keyCode) {
        return GE::Input::IsKeyPressed(static_cast<GE::KeyCode>(keyCode));
    });
    m.def("get_mouse_position", []() {
        auto [x, y] = GE::Input::GetMousePosition();
        return py::make_tuple(x, y);
    });

    // ── Transform ─────────────────────────────────────────────────────────────
    m.def("get_position", [](uint32_t id) -> glm::vec2 {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return {};
        GE::Entity e(static_cast<entt::entity>(id), scene);
        return e.GetComponent<GE::TransformComponent>().Position;
    });
    m.def("set_position", [](uint32_t id, glm::vec2 pos) {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return;
        GE::Entity e(static_cast<entt::entity>(id), scene);
        e.GetComponent<GE::TransformComponent>().Position = pos;
    });
    m.def("get_scale", [](uint32_t id) -> glm::vec2 {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return {};
        GE::Entity e(static_cast<entt::entity>(id), scene);
        return e.GetComponent<GE::TransformComponent>().Scale;
    });
    m.def("get_rotation", [](uint32_t id) -> float {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return 0.f;
        GE::Entity e(static_cast<entt::entity>(id), scene);
        return e.GetComponent<GE::TransformComponent>().Rotation;
    });
    m.def("set_rotation", [](uint32_t id, float rot) {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return;
        GE::Entity e(static_cast<entt::entity>(id), scene);
        e.GetComponent<GE::TransformComponent>().Rotation = rot;
    });

    // ── Rigidbody ─────────────────────────────────────────────────────────────
    m.def("get_velocity", [](uint32_t id) -> glm::vec2 {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return {};
        GE::Entity e(static_cast<entt::entity>(id), scene);
        auto& rb = e.GetComponent<GE::RigidBody2DComponent>();
        if (!rb.RuntimeBody) return {};
        const b2Vec2& v = rb.RuntimeBody->GetLinearVelocity();
        return { v.x, v.y };
    });
    m.def("set_velocity", [](uint32_t id, glm::vec2 vel) {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return;
        GE::Entity e(static_cast<entt::entity>(id), scene);
        auto& rb = e.GetComponent<GE::RigidBody2DComponent>();
        if (rb.RuntimeBody) rb.RuntimeBody->SetLinearVelocity({ vel.x, vel.y });
    });
    m.def("apply_impulse", [](uint32_t id, glm::vec2 impulse) {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return;
        GE::Entity e(static_cast<entt::entity>(id), scene);
        auto& rb = e.GetComponent<GE::RigidBody2DComponent>();
        if (rb.RuntimeBody)
            rb.RuntimeBody->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
    });

    // ── Scene queries ─────────────────────────────────────────────────────────
    m.def("find_entity", [](const std::string& name) -> uint32_t {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return UINT32_MAX;
        GE::Entity found = scene->FindByName(name);
        return found ? static_cast<uint32_t>(found) : UINT32_MAX;
    });

    // ── MLEnvironment bridge ──────────────────────────────────────────────────
    // get_ml_environment(entity_id) → the C++ MLEnvironment for that entity
    // Wrapped so Python can call .reset() / .step(action) on it
    py::class_<GE::MLEnvironment>(m, "MLEnvironment")
        .def("reset", &GE::MLEnvironment::Reset)
        .def("step",  &GE::MLEnvironment::Step)
        .def("observation_size", &GE::MLEnvironment::GetObservationSize)
        .def("action_size",      &GE::MLEnvironment::GetActionSize)
        .def("name",             &GE::MLEnvironment::GetName);

    m.def("get_ml_environment", [](uint32_t id) -> GE::MLEnvironment* {
        auto* scene = GE::PythonEngine::GetSceneContext();
        if (!scene) return nullptr;
        GE::Entity e(static_cast<entt::entity>(id), scene);
        if (!e.HasComponent<GE::MLEnvironmentComponent>()) return nullptr;
        return e.GetComponent<GE::MLEnvironmentComponent>().Environment.get();
    }, py::return_value_policy::reference);
}

namespace GE {
// Called from PythonEngine::Init() to force the linker to include this TU.
// Without this, MSVC strips the object file from the static library because
// PYBIND11_EMBEDDED_MODULE only registers via static init (no exported symbol).
void ForceEngineBindingsLink() {}
} // namespace GE
