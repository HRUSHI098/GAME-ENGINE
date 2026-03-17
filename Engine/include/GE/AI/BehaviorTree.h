#pragma once

#include "GE/Core/Base.h"
#include "GE/ECS/Entity.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>

namespace GE {

// ─── NodeStatus ───────────────────────────────────────────────────────────────
enum class BTStatus { Success, Failure, Running };

// ─── BTNode ───────────────────────────────────────────────────────────────────
class BTNode {
public:
    virtual ~BTNode() = default;
    virtual BTStatus Tick(Entity entity, f32 dt) = 0;
    virtual void Reset() {}
};

// ─── Composite nodes ─────────────────────────────────────────────────────────

// Sequence: ticks children left-to-right; returns Failure on first child Failure
class BTSequence : public BTNode {
public:
    explicit BTSequence(std::vector<Scope<BTNode>> children)
        : m_Children(std::move(children)) {}
    BTStatus Tick(Entity entity, f32 dt) override;
    void     Reset() override;
private:
    std::vector<Scope<BTNode>> m_Children;
    u32 m_CurrentChild = 0;
};

// Selector: ticks children left-to-right; returns Success on first child Success
class BTSelector : public BTNode {
public:
    explicit BTSelector(std::vector<Scope<BTNode>> children)
        : m_Children(std::move(children)) {}
    BTStatus Tick(Entity entity, f32 dt) override;
    void     Reset() override;
private:
    std::vector<Scope<BTNode>> m_Children;
    u32 m_CurrentChild = 0;
};

// Inverter: flips Success ↔ Failure of its single child
class BTInverter : public BTNode {
public:
    explicit BTInverter(Scope<BTNode> child) : m_Child(std::move(child)) {}
    BTStatus Tick(Entity entity, f32 dt) override;
private:
    Scope<BTNode> m_Child;
};

// ─── Leaf nodes ───────────────────────────────────────────────────────────────

// Condition: returns Success / Failure based on a predicate
class BTCondition : public BTNode {
public:
    using Predicate = std::function<bool(Entity)>;
    explicit BTCondition(Predicate pred) : m_Predicate(std::move(pred)) {}
    BTStatus Tick(Entity entity, f32 dt) override {
        return m_Predicate(entity) ? BTStatus::Success : BTStatus::Failure;
    }
private:
    Predicate m_Predicate;
};

// Action: executes a C++ lambda; returns its result
class BTAction : public BTNode {
public:
    using ActionFn = std::function<BTStatus(Entity, f32)>;
    explicit BTAction(ActionFn fn) : m_Fn(std::move(fn)) {}
    BTStatus Tick(Entity entity, f32 dt) override { return m_Fn(entity, dt); }
private:
    ActionFn m_Fn;
};

// PyLeaf: delegates tick to a named Python function in the entity's AIComponent
class BTPyLeaf : public BTNode {
public:
    explicit BTPyLeaf(std::string funcName) : m_FuncName(std::move(funcName)) {}
    BTStatus Tick(Entity entity, f32 dt) override;
private:
    std::string m_FuncName;
};

// ─── BehaviorTreeComponent ────────────────────────────────────────────────────
struct BehaviorTreeComponent {
    Scope<BTNode> Root = nullptr;

    // Convenience builder — returns a sequence node from a list of children
    static Scope<BTNode> MakeSequence(std::vector<Scope<BTNode>> children) {
        return MakeScope<BTSequence>(std::move(children));
    }
    static Scope<BTNode> MakeSelector(std::vector<Scope<BTNode>> children) {
        return MakeScope<BTSelector>(std::move(children));
    }
    static Scope<BTNode> MakeCondition(BTCondition::Predicate pred) {
        return MakeScope<BTCondition>(std::move(pred));
    }
    static Scope<BTNode> MakeAction(BTAction::ActionFn fn) {
        return MakeScope<BTAction>(std::move(fn));
    }
    static Scope<BTNode> MakePyLeaf(const std::string& funcName) {
        return MakeScope<BTPyLeaf>(funcName);
    }
    static Scope<BTNode> MakeInverter(Scope<BTNode> child) {
        return MakeScope<BTInverter>(std::move(child));
    }
};

} // namespace GE
