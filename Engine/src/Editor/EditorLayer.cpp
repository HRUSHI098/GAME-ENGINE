#include "GE/Editor/EditorLayer.h"
#include "GE/Core/Application.h"
#include "GE/Core/Log.h"
#include "GE/Renderer/Renderer2D.h"
#include "GE/Scene/SceneSerializer.h"
#include <imgui.h>
#include <filesystem>

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
    m_ImGuiLayer.SetBlockEvents(false);
    ApplyDarkTheme();
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

    // ── Render scene into viewport texture ────────────────────────────────────
    if (m_Scene) {
        int vpW = m_ViewportPanel.GetWidth();
        int vpH = m_ViewportPanel.GetHeight();
        if (vpW > 0 && vpH > 0) {
            m_Scene->OnViewportResize(static_cast<u32>(vpW), static_cast<u32>(vpH));
            Renderer2D::BeginViewport(vpW, vpH);
            m_Scene->OnRender();
            Renderer2D::EndViewport();
        }
    }

    // ── ImGui frame ───────────────────────────────────────────────────────────
    m_ImGuiLayer.Begin();
    DrawDockspace();
    DrawMainMenuBar();
    DrawToolbar();

    if (m_ShowViewport)   m_ViewportPanel.OnImGuiRender();
    if (m_ShowHierarchy) {
        m_HierarchyPanel.OnImGuiRender();
        m_PropertiesPanel.SetSelectedEntity(m_HierarchyPanel.GetSelectedEntity());
    }
    if (m_ShowProperties) m_PropertiesPanel.OnImGuiRender();
    if (m_ShowStats)      m_StatsPanel.OnImGuiRender(dt);
    if (m_ShowAssets)     m_AssetBrowser.OnImGuiRender();
    if (m_ShowConsole)    m_ConsolePanel.OnImGuiRender();

    m_ImGuiLayer.End();
}

// ─── Dark theme ───────────────────────────────────────────────────────────────
void EditorLayer::ApplyDarkTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 4.0f;
    s.FrameRounding     = 3.0f;
    s.ScrollbarRounding = 3.0f;
    s.GrabRounding      = 3.0f;
    s.TabRounding       = 3.0f;
    s.FramePadding      = ImVec2(6, 3);
    s.ItemSpacing       = ImVec2(8, 4);
    s.WindowPadding     = ImVec2(8, 8);
    s.IndentSpacing     = 18.0f;
    s.ScrollbarSize     = 13.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]           = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    c[ImGuiCol_ChildBg]            = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    c[ImGuiCol_PopupBg]            = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    c[ImGuiCol_Border]             = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
    c[ImGuiCol_FrameBg]            = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
    c[ImGuiCol_FrameBgHovered]     = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);
    c[ImGuiCol_FrameBgActive]      = ImVec4(0.22f, 0.23f, 0.24f, 1.00f);
    c[ImGuiCol_TitleBg]            = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    c[ImGuiCol_TitleBgActive]      = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    c[ImGuiCol_MenuBarBg]          = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    c[ImGuiCol_ScrollbarBg]        = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    c[ImGuiCol_ScrollbarGrab]      = ImVec4(0.31f, 0.31f, 0.33f, 1.00f);
    c[ImGuiCol_CheckMark]          = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    c[ImGuiCol_SliderGrab]         = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    c[ImGuiCol_Button]             = ImVec4(0.22f, 0.23f, 0.25f, 1.00f);
    c[ImGuiCol_ButtonHovered]      = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    c[ImGuiCol_ButtonActive]       = ImVec4(0.20f, 0.46f, 0.90f, 1.00f);
    c[ImGuiCol_Header]             = ImVec4(0.20f, 0.40f, 0.70f, 0.55f);
    c[ImGuiCol_HeaderHovered]      = ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
    c[ImGuiCol_HeaderActive]       = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    c[ImGuiCol_Separator]          = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
    c[ImGuiCol_Tab]                = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    c[ImGuiCol_TabHovered]         = ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
    c[ImGuiCol_TabActive]          = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.26f, 0.38f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    c[ImGuiCol_DockingPreview]     = ImVec4(0.28f, 0.56f, 1.00f, 0.70f);
#endif
    c[ImGuiCol_Text]               = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_TextDisabled]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
}

// ─── Dockspace ────────────────────────────────────────────────────────────────
void EditorLayer::DrawDockspace() {
#ifdef IMGUI_HAS_DOCK
    static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0, 0));
    ImGui::Begin("##Dockspace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(id, ImVec2(0, 0), dockFlags);
    ImGui::End();
#endif
}

// ─── Main menu bar ────────────────────────────────────────────────────────────
void EditorLayer::DrawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene",  "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                if (m_Scene) {
                    SceneSerializer s(m_Scene);
                    s.Deserialize("scenes/scene.json");
                }
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                if (m_Scene) {
                    std::filesystem::create_directories("scenes");
                    SceneSerializer s(m_Scene);
                    s.Serialize("scenes/scene.json");
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4"))
                Application::Get().Close();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Viewport",        nullptr, &m_ShowViewport);
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_ShowHierarchy);
            ImGui::MenuItem("Properties",      nullptr, &m_ShowProperties);
            ImGui::MenuItem("Renderer Stats",  nullptr, &m_ShowStats);
            ImGui::MenuItem("Asset Browser",   nullptr, &m_ShowAssets);
            ImGui::MenuItem("Console",         nullptr, &m_ShowConsole);
            ImGui::EndMenu();
        }

        char buf[64];
        snprintf(buf, sizeof(buf), "%.1f FPS  |  %.3f ms",
                 1.0f / (m_DeltaTime > 0 ? m_DeltaTime : 1e-4f),
                 m_DeltaTime * 1000.0f);
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(buf).x - 8.0f);
        ImGui::TextDisabled("%s", buf);

        ImGui::EndMainMenuBar();
    }
}

// ─── Toolbar ─────────────────────────────────────────────────────────────────
void EditorLayer::DrawToolbar() {
    ImGui::SetNextWindowPos(ImVec2(0, 18),     ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 36), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.12f, 1.0f));

    ImGui::Begin("##Toolbar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);

    float btnSize = ImGui::GetWindowHeight() - 4.0f;
    float avail   = ImGui::GetContentRegionAvail().x;
    float offset  = (avail - btnSize * 2.0f - 4.0f) * 0.5f;
    if (offset > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    bool playing = (m_State == EditorState::Play);
    bool paused  = (m_State == EditorState::Pause);

    // Play / Pause toggle
    ImGui::PushStyleColor(ImGuiCol_Button,
        playing ? ImVec4(0.15f, 0.55f, 0.15f, 1.0f) :
        paused  ? ImVec4(0.55f, 0.45f, 0.05f, 1.0f) :
                  ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
    if (ImGui::Button(playing ? "||" : ">", ImVec2(btnSize, btnSize))) {
        if (m_State == EditorState::Edit || m_State == EditorState::Pause)
            m_State = EditorState::Play;
        else
            m_State = EditorState::Pause;
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(playing ? "Pause" : "Play");

    ImGui::SameLine(0, 4);

    // Stop button
    ImGui::PushStyleColor(ImGuiCol_Button,
        (m_State != EditorState::Edit) ? ImVec4(0.55f, 0.15f, 0.15f, 1.0f)
                                       : ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
    if (ImGui::Button("[]", ImVec2(btnSize, btnSize)))
        m_State = EditorState::Edit;
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop");

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace GE
