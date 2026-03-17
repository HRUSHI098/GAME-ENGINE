#include "GE/AI/PythonEngine.h"
#include "GE/AI/ScriptWatcher.h"
#include "GE/Core/Log.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <filesystem>

namespace py = pybind11;
namespace fs = std::filesystem;

namespace GE {

// Defined in EngineBindings.cpp — ensures the linker includes that TU so that
// PYBIND11_EMBEDDED_MODULE(ge_engine) static registration actually runs.
void ForceEngineBindingsLink();

bool        PythonEngine::s_Initialised = false;
Scene*      PythonEngine::s_Scene       = nullptr;
std::string PythonEngine::s_ScriptsRoot;

void PythonEngine::Init(const std::string& scriptsRootPath) {
    if (s_Initialised) {
        GE_CORE_WARN("[PythonEngine] Already initialised.");
        return;
    }

    ForceEngineBindingsLink();   // ensure ge_engine module is registered
    s_ScriptsRoot = scriptsRootPath;

    py::initialize_interpreter();

    // Add scriptsRootPath to sys.path so `import patrol_agent` works
    try {
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("insert")(0, scriptsRootPath);

        // Also expose the ge_engine module (registered via PYBIND11_EMBEDDED_MODULE)
        py::module_::import("ge_engine");

        GE_CORE_INFO("[PythonEngine] Initialised. Scripts root: {}", scriptsRootPath);
        s_Initialised = true;
    } catch (const py::error_already_set& e) {
        GE_CORE_ERROR("[PythonEngine] Init error: {}", e.what());
    }

    // Start watching for script changes (hot-reload)
    ScriptWatcher::Init(scriptsRootPath, [](const std::string& mod) {
        PythonEngine::ReloadModule(mod);
    });
}

void PythonEngine::Shutdown() {
    if (!s_Initialised) return;
    ScriptWatcher::Shutdown();
    py::finalize_interpreter();
    s_Initialised = false;
    GE_CORE_INFO("[PythonEngine] Shut down.");
}

void PythonEngine::ExecFile(const std::string& path) {
    if (!s_Initialised) return;
    try {
        py::eval_file(path);
    } catch (const py::error_already_set& e) {
        GE_CORE_ERROR("[PythonEngine] ExecFile '{}' error: {}", path, e.what());
    }
}

py::object PythonEngine::ImportModule(const std::string& moduleName) {
    if (!s_Initialised) return py::none();
    try {
        return py::module_::import(moduleName.c_str());
    } catch (const py::error_already_set& e) {
        GE_CORE_ERROR("[PythonEngine] ImportModule '{}' error: {}", moduleName, e.what());
        return py::none();
    }
}

void PythonEngine::ReloadModule(const std::string& moduleName) {
    if (!s_Initialised) return;
    try {
        py::module_ sys      = py::module_::import("sys");
        py::object  modules  = sys.attr("modules");
        if (!modules.contains(moduleName.c_str())) {
            GE_CORE_WARN("[PythonEngine] ReloadModule: '{}' not loaded, skipping.", moduleName);
            return;
        }
        py::module_ importlib = py::module_::import("importlib");
        py::object  mod       = modules[moduleName.c_str()];
        importlib.attr("reload")(mod);
        GE_CORE_INFO("[PythonEngine] Hot-reloaded module '{}'.", moduleName);
    } catch (const py::error_already_set& e) {
        GE_CORE_ERROR("[PythonEngine] ReloadModule '{}' error: {}", moduleName, e.what());
    }
}

} // namespace GE
