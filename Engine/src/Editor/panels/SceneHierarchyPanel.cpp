#include "GE/Editor/panels/SceneHierarchyPanel.h"
#include "GE/ECS/Components.h"
#include "GE/Core/Log.h"

#include <imgui.h>
#include <cstring>

namespace GE {

void SceneHierarchyPanel::OnImGuiRender() {
    ImGui::SetNextWindowPos(ImVec2(0, 40),    ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, 380), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene Hierarchy");

    if (!m_Scene) { ImGui::Text("No active scene."); ImGui::End(); return; }

    // Only draw root entities (those without a parent)
    m_Scene->View<TagComponent>([&](auto entityHandle, TagComponent&) {
        Entity entity(entityHandle, m_Scene);
        // Skip non-root nodes — they are drawn recursively from their parent
        if (entity.HasComponent<RelationshipComponent>() &&
            entity.GetComponent<RelationshipComponent>().Parent != entt::null)
            return;
        DrawEntityNode(entity);
    });

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
        m_Selected = {};

    if (ImGui::BeginPopupContextWindow("##hier_ctx",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("Create Empty Entity"))
            m_Selected = m_Scene->CreateEntity("Empty Entity");
        ImGui::EndPopup();
    }

    ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
    auto& tag = entity.GetComponent<TagComponent>().Tag;

    bool hasChildren = entity.HasComponent<RelationshipComponent>() &&
                       !entity.GetComponent<RelationshipComponent>().Children.empty();

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow    |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
        (hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf);

    bool opened = ImGui::TreeNodeEx(
        reinterpret_cast<void*>(static_cast<u64>(static_cast<u32>(entity))),
        flags, "%s", tag.c_str());

    if (ImGui::IsItemClicked())
        m_Selected = entity;

    bool deleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Create Child")) {
            Entity child = m_Scene->CreateEntity("Child");
            child.SetParent(entity);
            m_Selected = child;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete Entity")) deleted = true;
        ImGui::EndPopup();
    }

    // Drag source — drag entity onto another to reparent
    if (ImGui::BeginDragDropSource()) {
        u32 id = static_cast<u32>(entity);
        ImGui::SetDragDropPayload("ENTITY", &id, sizeof(u32));
        ImGui::Text("%s", tag.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target — accept an entity being dragged onto this one
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
            u32 srcId = *static_cast<const u32*>(payload->Data);
            Entity src(static_cast<entt::entity>(srcId), m_Scene);
            if (src.IsValid() && src != entity)
                src.SetParent(entity);
        }
        ImGui::EndDragDropTarget();
    }

    if (opened) {
        if (hasChildren) {
            for (auto childHandle : entity.GetChildren()) {
                Entity child(childHandle, m_Scene);
                if (child.IsValid()) DrawEntityNode(child);
            }
        }
        ImGui::TreePop();
    }

    if (deleted) {
        if (m_Selected == entity) m_Selected = {};
        m_Scene->DestroyEntity(entity);
    }
}

} // namespace GE
