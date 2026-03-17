#include "GE/Editor/panels/PropertiesPanel.h"
#include "GE/ECS/Scene.h"
#include "GE/ECS/Components.h"
#include "GE/Physics/PhysicsComponents.h"
#include "GE/Audio/AudioComponents.h"
#include "GE/AI/AIComponent.h"
#include "GE/Scripting/ScriptComponent.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <functional>

namespace GE {

// ─── DrawComponent helper ─────────────────────────────────────────────────────
// Renders a collapsible header for a component type. Shows a remove button.
template<typename T, typename UIFunc>
static void DrawComponent(const std::string& label, Entity entity, UIFunc uiFunc) {
    if (!entity.HasComponent<T>()) return;

    constexpr ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen   |
        ImGuiTreeNodeFlags_Framed        |
        ImGuiTreeNodeFlags_SpanAvailWidth|
        ImGuiTreeNodeFlags_AllowItemOverlap|
        ImGuiTreeNodeFlags_FramePadding;

    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
    float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImGui::Separator();
    bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(T).hash_code()),
                                  flags, "%s", label.c_str());
    ImGui::PopStyleVar();

    // Remove component button on the right side
    ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
    if (ImGui::Button("-", ImVec2{ lineHeight, lineHeight })) {
        entity.RemoveComponent<T>();
        if (open) ImGui::TreePop();
        return;
    }

    if (open) {
        uiFunc(entity.GetComponent<T>());
        ImGui::TreePop();
    }
}

// ─── DragVec2 helper ──────────────────────────────────────────────────────────
static bool DragVec2(const char* label, glm::vec2& values, float speed = 0.1f) {
    return ImGui::DragFloat2(label, glm::value_ptr(values), speed);
}

// ─── PropertiesPanel ─────────────────────────────────────────────────────────
void PropertiesPanel::OnImGuiRender() {
    ImGui::SetNextWindowPos(ImVec2(1040, 40),  ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(240, 580),  ImGuiCond_FirstUseEver);
    ImGui::Begin("Properties");

    if (!m_Entity || !m_Entity.IsValid()) {
        ImGui::Text("No entity selected.");
        ImGui::End();
        return;
    }

    DrawComponents();

    // ── Add Component button ──────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    float btnWidth = 200.0f;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - btnWidth) * 0.5f);
    if (ImGui::Button("Add Component", { btnWidth, 0 }))
        ImGui::OpenPopup("AddComponent");

    if (ImGui::BeginPopup("AddComponent")) {
        if (!m_Entity.HasComponent<SpriteRendererComponent>())
            if (ImGui::MenuItem("Sprite Renderer"))
                m_Entity.AddComponent<SpriteRendererComponent>();

        if (!m_Entity.HasComponent<RigidBody2DComponent>())
            if (ImGui::MenuItem("Rigidbody 2D"))
                m_Entity.AddComponent<RigidBody2DComponent>();

        if (!m_Entity.HasComponent<BoxCollider2DComponent>())
            if (ImGui::MenuItem("Box Collider 2D"))
                m_Entity.AddComponent<BoxCollider2DComponent>();

        if (!m_Entity.HasComponent<CircleCollider2DComponent>())
            if (ImGui::MenuItem("Circle Collider 2D"))
                m_Entity.AddComponent<CircleCollider2DComponent>();

        if (!m_Entity.HasComponent<CameraComponent>())
            if (ImGui::MenuItem("Camera"))
                m_Entity.AddComponent<CameraComponent>();

        if (!m_Entity.HasComponent<AudioSourceComponent>())
            if (ImGui::MenuItem("Audio Source"))
                m_Entity.AddComponent<AudioSourceComponent>();

        if (!m_Entity.HasComponent<ScriptComponent>())
            if (ImGui::MenuItem("Script (C#)"))
                m_Entity.AddComponent<ScriptComponent>();

        if (!m_Entity.HasComponent<AIComponent>())
            if (ImGui::MenuItem("AI Script (Python)"))
                m_Entity.AddComponent<AIComponent>();

        ImGui::EndPopup();
    }

    ImGui::End();
}

void PropertiesPanel::DrawComponents() {
    // ── Tag ───────────────────────────────────────────────────────────────────
    if (m_Entity.HasComponent<TagComponent>()) {
        auto& tag = m_Entity.GetComponent<TagComponent>().Tag;
        char buf[256];
        memset(buf, 0, sizeof(buf));
        strncpy(buf, tag.c_str(), sizeof(buf) - 1); buf[sizeof(buf)-1] = '\0';
        if (ImGui::InputText("##Tag", buf, sizeof(buf)))
            tag = buf;
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1.0f);
    ImGui::Text("ID: %u", static_cast<u32>(m_Entity));
    ImGui::PopItemWidth();

    // ── Transform ─────────────────────────────────────────────────────────────
    DrawComponent<TransformComponent>("Transform", m_Entity, [](TransformComponent& tc) {
        DragVec2("Position", tc.Position, 0.05f);
        DragVec2("Scale",    tc.Scale,    0.05f);
        float deg = glm::degrees(tc.Rotation);
        if (ImGui::DragFloat("Rotation (°)", &deg, 0.5f))
            tc.Rotation = glm::radians(deg);
    });

    // ── Sprite Renderer ───────────────────────────────────────────────────────
    DrawComponent<SpriteRendererComponent>("Sprite Renderer", m_Entity, [](SpriteRendererComponent& sc) {
        ImGui::ColorEdit4("Color", glm::value_ptr(sc.Color));
        ImGui::DragInt("Z Order", &sc.ZOrder);
        if (sc.Texture)
            ImGui::Text("Texture: loaded (%dx%d)", sc.Texture->GetWidth(), sc.Texture->GetHeight());
        else
            ImGui::TextDisabled("No texture");
    });

    // ── Camera ────────────────────────────────────────────────────────────────
    DrawComponent<CameraComponent>("Camera", m_Entity, [&](CameraComponent& cc) {
        ImGui::Checkbox("Primary",            &cc.Primary);
        ImGui::Checkbox("Fixed Aspect Ratio", &cc.FixedAspectRatio);

        ImGui::Separator();
        ImGui::Text("Projection");

        float size = cc.OrthographicSize;
        if (ImGui::DragFloat("Ortho Size", &size, 0.05f, 0.1f, 100.0f, "%.2f")) {
            cc.OrthographicSize = size;
            // Recalculate projection from current viewport aspect
            Scene* scene = m_Entity.GetScene();
            if (scene) {
                float aspect = (float)scene->GetViewportWidth() / (float)scene->GetViewportHeight();
                cc.RecalcProjection(aspect);
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Half-height in world units. Smaller = zoomed in.");

        ImGui::Separator();
        ImGui::Text("Environment");
        ImGui::ColorEdit4("Background", glm::value_ptr(cc.BackgroundColor));
    });

    // ── Rigidbody 2D ──────────────────────────────────────────────────────────
    DrawComponent<RigidBody2DComponent>("Rigidbody 2D", m_Entity, [](RigidBody2DComponent& rb) {
        const char* types[] = { "Static", "Dynamic", "Kinematic" };
        int current = static_cast<int>(rb.Type);
        if (ImGui::Combo("Body Type", &current, types, 3))
            rb.Type = static_cast<RigidBody2DComponent::BodyType>(current);
        ImGui::Checkbox("Fixed Rotation", &rb.FixedRotation);
    });

    // ── Box Collider 2D ───────────────────────────────────────────────────────
    DrawComponent<BoxCollider2DComponent>("Box Collider 2D", m_Entity, [](BoxCollider2DComponent& bc) {
        DragVec2("Offset",    bc.Offset, 0.01f);
        DragVec2("Size",      bc.Size,   0.01f);
        ImGui::DragFloat("Density",     &bc.Density,     0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Friction",    &bc.Friction,    0.01f, 0.0f,   1.0f);
        ImGui::DragFloat("Restitution", &bc.Restitution, 0.01f, 0.0f,   1.0f);
    });

    // ── Circle Collider 2D ────────────────────────────────────────────────────
    DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", m_Entity, [](CircleCollider2DComponent& cc) {
        DragVec2("Offset", cc.Offset, 0.01f);
        ImGui::DragFloat("Radius",      &cc.Radius,      0.01f, 0.001f, 100.0f);
        ImGui::DragFloat("Density",     &cc.Density,     0.01f, 0.0f,   100.0f);
        ImGui::DragFloat("Friction",    &cc.Friction,    0.01f, 0.0f,     1.0f);
        ImGui::DragFloat("Restitution", &cc.Restitution, 0.01f, 0.0f,     1.0f);
    });

    // ── Audio Source ──────────────────────────────────────────────────────────
    DrawComponent<AudioSourceComponent>("Audio Source", m_Entity, [](AudioSourceComponent& asc) {
        ImGui::DragFloat("Volume",     &asc.Volume,     0.01f, 0.0f, 1.0f);
        ImGui::Checkbox("Loop",        &asc.Loop);
        ImGui::Checkbox("Play On Awake", &asc.PlayOnAwake);
        ImGui::Text("Channel: %d", asc.Channel);
    });

    // ── C# Script ─────────────────────────────────────────────────────────────
    DrawComponent<ScriptComponent>("Script (C#)", m_Entity, [](ScriptComponent& sc) {
        char buf[128] = {};
        strncpy(buf, sc.ClassName.c_str(), sizeof(buf) - 1); buf[sizeof(buf)-1] = '\0';
        if (ImGui::InputText("Class Name", buf, sizeof(buf)))
            sc.ClassName = buf;
    });

    // ── Python AI Script ──────────────────────────────────────────────────────
    DrawComponent<AIComponent>("AI Script (Python)", m_Entity, [](AIComponent& ai) {
        char modBuf[128] = {}, clsBuf[128] = {};
        strncpy(modBuf, ai.ModuleName.c_str(), sizeof(modBuf) - 1); modBuf[sizeof(modBuf)-1] = '\0';
        strncpy(clsBuf, ai.ClassName.c_str(),  sizeof(clsBuf) - 1); clsBuf[sizeof(clsBuf)-1] = '\0';
        if (ImGui::InputText("Module", modBuf, sizeof(modBuf))) ai.ModuleName = modBuf;
        if (ImGui::InputText("Class",  clsBuf, sizeof(clsBuf))) ai.ClassName  = clsBuf;
        ImGui::Text("Instance: %s", ai.PyInstance ? "active" : "none");
    });
}

} // namespace GE
