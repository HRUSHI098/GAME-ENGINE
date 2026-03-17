#pragma once

#include "GE/Core/Base.h"
#include <string>
#include <functional>

// Forward-declare pybind11 types to avoid leaking Python.h into all headers
namespace pybind11 { class object; class module_; }
namespace py = pybind11;

namespace GE {

class Scene;

// ─── PythonEngine ─────────────────────────────────────────────────────────────
// Manages the embedded Python interpreter lifetime and the `ge` engine module.
// One interpreter per process — do NOT call Init() more than once.
class PythonEngine {
public:
    static void Init(const std::string& scriptsRootPath);
    static void Shutdown();

    // Run a .py file in the main interpreter (global scope)
    static void ExecFile(const std::string& path);

    // Import a Python module by dotted name (searches scriptsRootPath first)
    // Returns a py::object wrapping the module, or a null object on failure.
    static py::object ImportModule(const std::string& moduleName);

    // Call a free function in an already-imported module
    // args are forwarded as Python positional args
    template<typename... Args>
    static py::object Call(const py::object& module,
                           const std::string& funcName,
                           Args&&... args);

    // Set the active scene (used by Python bindings to resolve entity IDs)
    static void SetSceneContext(Scene* scene) { s_Scene = scene; }
    static Scene* GetSceneContext()           { return s_Scene; }

    static bool IsInitialised() { return s_Initialised; }

private:
    static bool    s_Initialised;
    static Scene*  s_Scene;
    static std::string s_ScriptsRoot;
};

} // namespace GE
