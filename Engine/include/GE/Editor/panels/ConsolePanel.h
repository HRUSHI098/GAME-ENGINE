#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <imgui.h>

namespace GE {

enum class LogLevel { Trace, Info, Warn, Error, Fatal };

struct ConsoleEntry {
    LogLevel    Level;
    std::string Message;
};

// Static sink — engine log macros call ConsolePanel::Push()
class ConsolePanel {
public:
    static void Push(LogLevel level, const std::string& msg);

    void OnImGuiRender();
    void Clear() { std::lock_guard<std::mutex> lk(s_Mutex); s_Entries.clear(); }

private:
    static std::vector<ConsoleEntry> s_Entries;
    static std::mutex                s_Mutex;

    bool m_ScrollToBottom = true;
    bool m_FilterWarn     = false;
};

} // namespace GE
