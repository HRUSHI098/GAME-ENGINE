#pragma once

#include "GE/Core/Base.h"
#include "GE/ECS/Entity.h"

namespace GE {

class PropertiesPanel {
public:
    PropertiesPanel() = default;

    void SetSelectedEntity(Entity e) { m_Entity = e; }
    void OnImGuiRender();

private:
    void DrawComponents();

    Entity m_Entity = {};
};

} // namespace GE
