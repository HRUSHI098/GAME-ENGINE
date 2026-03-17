#pragma once

#include "GE/Core/Base.h"
#include <string>
#include <memory>

namespace pybind11 { class object; }
namespace py = pybind11;

namespace GE {

// ─── AIComponent ─────────────────────────────────────────────────────────────
// Attach a Python AI script to an entity.
//
// The Python class must implement:
//   def on_create(self, entity_id: int)  -> None
//   def on_update(self, entity_id: int, dt: float) -> None
//   def on_destroy(self, entity_id: int) -> None   (optional)
//
// Example attachment:
//   entity.AddComponent<AIComponent>("patrol_agent", "PatrolAgent");
struct AIComponent {
    std::string ModuleName;   // Python module file (relative to scripts root)
    std::string ClassName;    // Python class inside that module

    // Runtime — owned Python object instance (null until scene start)
    // Stored as void* to avoid Python.h in this header; cast in AISystem
    void* PyInstance = nullptr;

    AIComponent() = default;
    AIComponent(std::string module, std::string cls)
        : ModuleName(std::move(module)), ClassName(std::move(cls)) {}
};

} // namespace GE
