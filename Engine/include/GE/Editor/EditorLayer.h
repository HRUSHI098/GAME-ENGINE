#pragma once

#include "GE/Core/Layer.h"
#include "GE/Editor/ImGuiLayer.h"
#include "GE/Editor/panels/SceneHierarchyPanel.h"
#include "GE/Editor/panels/PropertiesPanel.h"
#include "GE/Editor/panels/StatsPanel.h"
#include "GE/ECS/Scene.h"

namespace GE {

// ─── EditorLayer ─────────────────────────────────────────────────────────────
// Push this as an overlay on top of the game layer in debug/editor builds.
// It owns ImGui and all editor panels.
class EditorLayer : public Layer {
public:
    explicit EditorLayer(Scene* scene = nullptr);
    ~EditorLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(f32 deltaTime) override;
    void OnEvent(Event& e) override;
    void OnImGuiRender() override {}   // called by EditorLayer::OnUpdate itself

    void SetScene(Scene* scene);

    static EditorLayer* Get()  { return s_Instance; }

private:
    static EditorLayer* s_Instance;
    void DrawMainMenuBar();
    void DrawDockspace();

    ImGuiLayer           m_ImGuiLayer;
    SceneHierarchyPanel  m_HierarchyPanel;
    PropertiesPanel      m_PropertiesPanel;
    StatsPanel           m_StatsPanel;

    Scene*  m_Scene     = nullptr;
    f32     m_DeltaTime = 0.0f;

    bool m_ShowHierarchy  = true;
    bool m_ShowProperties = true;
    bool m_ShowStats      = true;
};

} // namespace GE
