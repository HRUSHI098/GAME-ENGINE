#include "GE/Editor/panels/ConsolePanel.h"
#include <imgui.h>

namespace GE {

std::vector<ConsoleEntry> ConsolePanel::s_Entries;
std::mutex                ConsolePanel::s_Mutex;

void ConsolePanel::Push(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lk(s_Mutex);
    s_Entries.push_back({ level, msg });
    if (s_Entries.size() > 500)
        s_Entries.erase(s_Entries.begin());
}

void ConsolePanel::OnImGuiRender() {
    ImGui::SetNextWindowPos(ImVec2(220, 620),  ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(820, 160),  ImGuiCond_FirstUseEver);
    ImGui::Begin("Console");

    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    ImGui::Checkbox("Warnings+", &m_FilterWarn);
    ImGui::Separator();

    ImGui::BeginChild("##log", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lk(s_Mutex);
    for (auto& entry : s_Entries) {
        if (m_FilterWarn && entry.Level < LogLevel::Warn) continue;

        ImVec4 col;
        switch (entry.Level) {
            case LogLevel::Trace: col = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); break;
            case LogLevel::Info:  col = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); break;
            case LogLevel::Warn:  col = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
            case LogLevel::Error: col = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break;
            case LogLevel::Fatal: col = ImVec4(1.0f, 0.0f, 0.5f, 1.0f); break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(entry.Message.c_str());
        ImGui::PopStyleColor();
    }

    if (m_ScrollToBottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

} // namespace GE
