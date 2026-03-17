#pragma once
#include "GE/Core/Base.h"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>

namespace GE {

// ─── ScriptWatcher ────────────────────────────────────────────────────────────
// Polls script files for modification time changes.
// When a change is detected it calls the registered callback so the
// PythonEngine can reload the affected module.
class ScriptWatcher {
public:
    using ReloadCallback = std::function<void(const std::string& moduleName)>;

    static void Init(const std::string& scriptsDir, ReloadCallback cb);
    static void Shutdown();
    static void OnUpdate();   // call once per frame (cheap stat check)

private:
    static std::string                                           s_Dir;
    static ReloadCallback                                        s_Callback;
    static std::unordered_map<std::string, std::filesystem::file_time_type> s_Timestamps;
};

} // namespace GE
