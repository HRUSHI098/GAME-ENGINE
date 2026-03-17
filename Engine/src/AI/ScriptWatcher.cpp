#include "GE/AI/ScriptWatcher.h"
#include "GE/Core/Log.h"

namespace GE {

std::string                                           ScriptWatcher::s_Dir;
ScriptWatcher::ReloadCallback                         ScriptWatcher::s_Callback;
std::unordered_map<std::string, std::filesystem::file_time_type> ScriptWatcher::s_Timestamps;

void ScriptWatcher::Init(const std::string& scriptsDir, ReloadCallback cb) {
    s_Dir      = scriptsDir;
    s_Callback = std::move(cb);
    s_Timestamps.clear();

    // Snapshot current timestamps
    if (!std::filesystem::exists(s_Dir)) return;
    for (auto& entry : std::filesystem::recursive_directory_iterator(s_Dir)) {
        if (entry.path().extension() == ".py")
            s_Timestamps[entry.path().string()] = entry.last_write_time();
    }
    GE_CORE_INFO("ScriptWatcher: watching '{}'", s_Dir);
}

void ScriptWatcher::Shutdown() {
    s_Timestamps.clear();
    s_Callback = nullptr;
}

void ScriptWatcher::OnUpdate() {
    if (!std::filesystem::exists(s_Dir)) return;

    for (auto& entry : std::filesystem::recursive_directory_iterator(s_Dir)) {
        if (entry.path().extension() != ".py") continue;

        auto path  = entry.path().string();
        auto mtime = entry.last_write_time();

        auto it = s_Timestamps.find(path);
        if (it == s_Timestamps.end()) {
            s_Timestamps[path] = mtime;
            continue;
        }

        if (mtime != it->second) {
            it->second = mtime;
            // Derive module name from filename (strip path + .py)
            std::string mod = entry.path().stem().string();
            GE_CORE_INFO("ScriptWatcher: '{}' changed — reloading module '{}'", path, mod);
            if (s_Callback) s_Callback(mod);
        }
    }
}

} // namespace GE
