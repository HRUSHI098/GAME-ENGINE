#include "GE/AI/AIComponent.h"
#include "GE/AI/BehaviorTree.h"
#include "GE/AI/MLEnvironment.h"
#include "GE/AI/PythonEngine.h"
#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"
#include "GE/Core/Log.h"

#include <pybind11/embed.h>

namespace py = pybind11;

namespace GE {

// ─── BehaviorTree node implementations ───────────────────────────────────────
BTStatus BTSequence::Tick(Entity entity, f32 dt) {
    while (m_CurrentChild < m_Children.size()) {
        BTStatus s = m_Children[m_CurrentChild]->Tick(entity, dt);
        if (s == BTStatus::Failure) { m_CurrentChild = 0; return BTStatus::Failure; }
        if (s == BTStatus::Running) return BTStatus::Running;
        ++m_CurrentChild;
    }
    m_CurrentChild = 0;
    return BTStatus::Success;
}
void BTSequence::Reset() {
    m_CurrentChild = 0;
    for (auto& c : m_Children) c->Reset();
}

BTStatus BTSelector::Tick(Entity entity, f32 dt) {
    while (m_CurrentChild < m_Children.size()) {
        BTStatus s = m_Children[m_CurrentChild]->Tick(entity, dt);
        if (s == BTStatus::Success) { m_CurrentChild = 0; return BTStatus::Success; }
        if (s == BTStatus::Running) return BTStatus::Running;
        ++m_CurrentChild;
    }
    m_CurrentChild = 0;
    return BTStatus::Failure;
}
void BTSelector::Reset() {
    m_CurrentChild = 0;
    for (auto& c : m_Children) c->Reset();
}

BTStatus BTInverter::Tick(Entity entity, f32 dt) {
    BTStatus s = m_Child->Tick(entity, dt);
    if (s == BTStatus::Success) return BTStatus::Failure;
    if (s == BTStatus::Failure) return BTStatus::Success;
    return BTStatus::Running;
}

BTStatus BTPyLeaf::Tick(Entity entity, f32 dt) {
    if (!PythonEngine::IsInitialised()) return BTStatus::Failure;
    if (!entity.HasComponent<AIComponent>())  return BTStatus::Failure;

    auto& ai = entity.GetComponent<AIComponent>();
    if (!ai.PyInstance) return BTStatus::Failure;

    try {
        auto* obj = reinterpret_cast<py::object*>(ai.PyInstance);
        py::object result = obj->attr(m_FuncName.c_str())(
            static_cast<u32>(entity), dt);

        // Python function should return "success", "failure", or "running"
        std::string status = result.cast<std::string>();
        if (status == "success") return BTStatus::Success;
        if (status == "running") return BTStatus::Running;
        return BTStatus::Failure;
    } catch (const py::error_already_set& e) {
        GE_CORE_ERROR("[BTPyLeaf] '{}' error: {}", m_FuncName, e.what());
        return BTStatus::Failure;
    }
}

// ─── AI scene lifecycle (called from Scene) ───────────────────────────────────
void AISystem_OnSceneStart(Scene* scene) {
    if (!PythonEngine::IsInitialised()) return;

    PythonEngine::SetSceneContext(scene);

    scene->View<AIComponent>([&](auto entity, AIComponent& ai) {
        try {
            py::object module = PythonEngine::ImportModule(ai.ModuleName);
            if (module.is_none()) return;

            py::object cls      = module.attr(ai.ClassName.c_str());
            py::object instance = cls();

            // Store the py::object on the heap (lifetime = entity's)
            ai.PyInstance = new py::object(std::move(instance));

            // Call on_create(entity_id)
            auto* obj = reinterpret_cast<py::object*>(ai.PyInstance);
            obj->attr("on_create")(static_cast<u32>(entity));

            GE_CORE_INFO("[AI] Script '{}.{}' attached to entity {}.",
                         ai.ModuleName, ai.ClassName, static_cast<u32>(entity));
        } catch (const py::error_already_set& e) {
            GE_CORE_ERROR("[AI] Failed to attach '{}.{}': {}",
                          ai.ModuleName, ai.ClassName, e.what());
        }
    });
}

void AISystem_OnUpdate(Scene* scene, f32 dt) {
    if (!PythonEngine::IsInitialised()) return;

    // Tick Python AI scripts
    scene->View<AIComponent>([&](auto entity, AIComponent& ai) {
        if (!ai.PyInstance) return;
        try {
            auto* obj = reinterpret_cast<py::object*>(ai.PyInstance);
            obj->attr("on_update")(static_cast<u32>(entity), dt);
        } catch (const py::error_already_set& e) {
            GE_CORE_ERROR("[AI] on_update error on entity {}: {}",
                          static_cast<u32>(entity), e.what());
        }
    });

    // Tick Behavior Trees
    scene->View<BehaviorTreeComponent>([&](auto entity, BehaviorTreeComponent& btc) {
        if (btc.Root)
            btc.Root->Tick(Entity(entity, scene), dt);
    });

    // Tick ML environments
    scene->View<MLEnvironmentComponent>([&](auto, MLEnvironmentComponent& mlc) {
        if (mlc.Environment)
            mlc.Environment->OnUpdate(dt);
    });
}

void AISystem_OnSceneStop(Scene* scene) {
    scene->View<AIComponent>([](auto entity, AIComponent& ai) {
        if (!ai.PyInstance) return;
        try {
            auto* obj = reinterpret_cast<py::object*>(ai.PyInstance);
            if (py::hasattr(*obj, "on_destroy"))
                obj->attr("on_destroy")(static_cast<u32>(entity));
        } catch (...) {}
        delete reinterpret_cast<py::object*>(ai.PyInstance);
        ai.PyInstance = nullptr;
    });
    PythonEngine::SetSceneContext(nullptr);
}

} // namespace GE
