#include "GE/Editor/panels/SceneHierarchyPanel.h"
#include "GE/ECS/Components.h"
#include "GE/Core/Log.h"

#include <imgui.h>
#include <cstring>

namespace GE {

void SceneHierarchyPanel::OnImGuiRender() {
    ImGui::Begin("Scene Hierarchy");

    if (!m_Scene) { ImGui::Text("No active scene."); ImGui::End(); return; }

    // ── List all entities ─────────────────────────────────────────────────────
    m_Scene->View<TagComponent>([&](auto entityHandle, TagComponent&) {
        Entity entity(entityHandle, m_Scene);
        DrawEntityNode(entity);
    });

    // ── Deselect on blank click ───────────────────────────────────────────────
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
        m_Selected = {};

    // ── Right-click on blank space ────────────────────────────────────────────
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

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow      |
        ImGuiTreeNodeFlags_SpanAvailWidth   |
        (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
        ImGuiTreeNodeFlags_Leaf;            // no children yet (scene graph Phase later)

    bool opened = ImGui::TreeNodeEx(
        reinterpret_cast<void*>(static_cast<u64>(static_cast<u32>(entity))),
        flags, "%s", tag.c_str());

    if (ImGui::IsItemClicked())
        m_Selected = entity;

    bool deleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Rename")) { /* handled via inspector */ }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete Entity")) deleted = true;
        ImGui::EndPopup();
    }

    if (opened) ImGui::TreePop();

    if (deleted) {
        if (m_Selected == entity) m_Selected = {};
        m_Scene->DestroyEntity(entity);
    }
}

} // namespace GE
