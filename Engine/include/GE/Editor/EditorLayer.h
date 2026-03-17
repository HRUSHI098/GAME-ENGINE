#pragma once

#include "GE/Core/Layer.h"
#include "GE/Editor/ImGuiLayer.h"
#include "GE/Editor/panels/SceneHierarchyPanel.h"
#include "GE/Editor/panels/PropertiesPanel.h"
#include "GE/Editor/panels/StatsPanel.h"
#include "GE/Editor/panels/ViewportPanel.h"
#include "GE/Editor/panels/AssetBrowserPanel.h"
#include "GE/Editor/panels/ConsolePanel.h"
#include "GE/ECS/Scene.h"

namespace GE {

enum class EditorState { Edit, Play, Pause };

// ─── EditorLayer ─────────────────────────────────────────────────────────────
class EditorLayer : public Layer {
public:
    explicit EditorLayer(Scene* scene = nullptr);
    ~EditorLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(f32 deltaTime) override;
    void OnEvent(Event& e) override;
    void OnImGuiRender() override {}

    void SetScene(Scene* scene);

    static EditorLayer*  Get()       { return s_Instance; }
    static bool          IsPlaying() { return s_Instance && s_Instance->m_State == EditorState::Play; }
    static bool          IsPaused()  { return s_Instance && s_Instance->m_State == EditorState::Pause; }

private:
    static EditorLayer* s_Instance;

    void ApplyDarkTheme();
    void DrawMainMenuBar();
    void DrawDockspace();
    void DrawToolbar();

    ImGuiLayer           m_ImGuiLayer;
    SceneHierarchyPanel  m_HierarchyPanel;
    PropertiesPanel      m_PropertiesPanel;
    StatsPanel           m_StatsPanel;
    ViewportPanel        m_ViewportPanel;
    AssetBrowserPanel    m_AssetBrowser;
    ConsolePanel         m_ConsolePanel;

    Scene*       m_Scene     = nullptr;
    f32          m_DeltaTime = 0.0f;
    EditorState  m_State     = EditorState::Edit;

    bool m_ShowHierarchy  = true;
    bool m_ShowProperties = true;
    bool m_ShowStats      = true;
    bool m_ShowViewport   = true;
    bool m_ShowAssets     = true;
    bool m_ShowConsole    = true;
};

} // namespace GE
