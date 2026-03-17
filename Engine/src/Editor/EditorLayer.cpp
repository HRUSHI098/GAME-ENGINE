#include "GE/Editor/EditorLayer.h"
#include "GE/Core/Application.h"
#include "GE/Core/Log.h"

#include <imgui.h>

namespace GE {

EditorLayer* EditorLayer::s_Instance = nullptr;

EditorLayer::EditorLayer(Scene* scene)
    : Layer("EditorLayer"), m_Scene(scene) {
    m_HierarchyPanel.SetScene(scene);
}

void EditorLayer::SetScene(Scene* scene) {
    m_Scene = scene;
    m_HierarchyPanel.SetScene(scene);
}

void EditorLayer::OnAttach() {
    s_Instance = this;
    m_ImGuiLayer.OnAttach();
    m_ImGuiLayer.SetBlockEvents(false);   // let game receive input too
    GE_CORE_INFO("EditorLayer attached.");
}

void EditorLayer::OnDetach() {
    m_ImGuiLayer.OnDetach();
    s_Instance = nullptr;
}

void EditorLayer::OnEvent(Event& e) {
    m_ImGuiLayer.OnEvent(e);
}

void EditorLayer::OnUpdate(f32 dt) {
    m_DeltaTime = dt;
    m_ImGuiLayer.Begin();

    DrawDockspace();
    DrawMainMenuBar();

    if (m_ShowHierarchy) {
        m_HierarchyPanel.OnImGuiRender();
        // Sync selection → Properties panel
        m_PropertiesPanel.SetSelectedEntity(m_HierarchyPanel.GetSelectedEntity());
    }

    if (m_ShowProperties)
        m_PropertiesPanel.OnImGuiRender();

    if (m_ShowStats)
        m_StatsPanel.OnImGuiRender(dt);

    m_ImGuiLayer.End();
}

// ─── Main menu bar ────────────────────────────────────────────────────────────
void EditorLayer::DrawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene",  "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4"))
                Application::Get().Close();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_ShowHierarchy);
            ImGui::MenuItem("Properties",      nullptr, &m_ShowProperties);
            ImGui::MenuItem("Renderer Stats",  nullptr, &m_ShowStats);
            ImGui::EndMenu();
        }

        // Right-aligned frame info
        char buf[64];
        snprintf(buf, sizeof(buf), "%.1f FPS  |  %.3f ms",
                 1.0f / (m_DeltaTime > 0 ? m_DeltaTime : 1e-4f),
                 m_DeltaTime * 1000.0f);
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(buf).x - 8.0f);
        ImGui::TextDisabled("%s", buf);

        ImGui::EndMainMenuBar();
    }
}

// ─── Dockspace ────────────────────────────────────────────────────────────────
void EditorLayer::DrawDockspace() {
#ifdef IMGUI_HAS_DOCK
    static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("##Dockspace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(id, ImVec2(0, 0), dockFlags);
    ImGui::End();
#endif
}

} // namespace GE
