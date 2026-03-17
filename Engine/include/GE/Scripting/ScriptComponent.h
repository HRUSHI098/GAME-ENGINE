#pragma once

#include "GE/Core/Base.h"
#include <string>

namespace GE {

// ─── ScriptComponent ──────────────────────────────────────────────────────────
// Attach a C# class to an entity by name.
// The engine resolves the class from the loaded app assembly at scene start.
struct ScriptComponent {
    std::string ClassName;   // e.g. "PlayerController" (namespace-qualified optional)

    ScriptComponent() = default;
    explicit ScriptComponent(std::string className)
        : ClassName(std::move(className)) {}
};

} // namespace GE
