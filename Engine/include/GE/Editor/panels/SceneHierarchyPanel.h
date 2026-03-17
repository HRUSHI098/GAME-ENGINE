#pragma once

#include "GE/Core/Base.h"
#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"

namespace GE {

class SceneHierarchyPanel {
public:
    SceneHierarchyPanel() = default;
    explicit SceneHierarchyPanel(Scene* scene) : m_Scene(scene) {}

    void SetScene(Scene* scene) { m_Scene = scene; m_Selected = {}; }
    void OnImGuiRender();

    Entity GetSelectedEntity() const { return m_Selected; }
    void   SetSelectedEntity(Entity e) { m_Selected = e; }

private:
    void DrawEntityNode(Entity entity);

    Scene*  m_Scene    = nullptr;
    Entity  m_Selected = {};
};

} // namespace GE
