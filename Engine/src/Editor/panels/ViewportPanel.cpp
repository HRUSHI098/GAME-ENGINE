#include "GE/Editor/panels/ViewportPanel.h"
#include "GE/Renderer/Renderer2D.h"
#include <imgui.h>
#include <SDL2/SDL.h>

namespace GE {

void ViewportPanel::OnImGuiRender() {
    ImGui::SetNextWindowPos(ImVec2(220, 40),   ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(820, 580),  ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport");

    m_Hovered = ImGui::IsWindowHovered();
    m_Focused = ImGui::IsWindowFocused();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    int w = static_cast<int>(avail.x > 1 ? avail.x : 1);
    int h = static_cast<int>(avail.y > 1 ? avail.y : 1);
    m_Width  = w;
    m_Height = h;

    SDL_Texture* tex = static_cast<SDL_Texture*>(Renderer2D::GetViewportTexture());
    if (tex)
        ImGui::Image(reinterpret_cast<ImTextureID>(tex), avail);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace GE
