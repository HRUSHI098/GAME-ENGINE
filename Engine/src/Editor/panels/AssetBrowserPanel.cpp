#include "GE/Editor/panels/AssetBrowserPanel.h"
#include <imgui.h>
#include <algorithm>

namespace GE {

AssetBrowserPanel::AssetBrowserPanel(const std::string& assetsRoot)
    : m_Root(assetsRoot), m_Current(assetsRoot) {}

void AssetBrowserPanel::OnImGuiRender() {
    ImGui::SetNextWindowPos(ImVec2(0, 580),    ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, 200),  ImGuiCond_FirstUseEver);
    ImGui::Begin("Asset Browser");

    // Back button
    if (m_Current != m_Root) {
        if (ImGui::Button("<- Back"))
            m_Current = m_Current.parent_path();
        ImGui::SameLine();
    }
    ImGui::TextDisabled("%s", m_Current.string().c_str());
    ImGui::Separator();

    if (!std::filesystem::exists(m_Current)) {
        ImGui::TextDisabled("(assets folder not found)");
        ImGui::End();
        return;
    }

    // Collect and sort entries: dirs first, then files
    std::vector<std::filesystem::directory_entry> dirs, files;
    for (auto& entry : std::filesystem::directory_iterator(m_Current)) {
        if (entry.is_directory()) dirs.push_back(entry);
        else                      files.push_back(entry);
    }
    auto byName = [](auto& a, auto& b){ return a.path().filename() < b.path().filename(); };
    std::sort(dirs.begin(),  dirs.end(),  byName);
    std::sort(files.begin(), files.end(), byName);

    for (auto& entry : dirs) {
        std::string label = "[DIR]  " + entry.path().filename().string();
        if (ImGui::Selectable(label.c_str()))
            m_Current = entry.path();
    }
    for (auto& entry : files) {
        std::string name = entry.path().filename().string();
        std::string label = "       " + name;
        ImGui::Selectable(label.c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", entry.path().string().c_str());
    }

    ImGui::End();
}

} // namespace GE
