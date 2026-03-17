#include "GE/Scene/SceneSerializer.h"
#include "GE/ECS/Entity.h"
#include "GE/ECS/Components.h"
#include "GE/Physics/PhysicsComponents.h"
#include "GE/Audio/AudioComponents.h"
#include "GE/AI/AIComponent.h"
#include "GE/Core/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace GE {

// ─── Helpers ──────────────────────────────────────────────────────────────────
static json Vec2Json(const glm::vec2& v) { return { v.x, v.y }; }
static json Vec4Json(const glm::vec4& v) { return { v.r, v.g, v.b, v.a }; }
static glm::vec2 JsonVec2(const json& j) { return { j[0].get<float>(), j[1].get<float>() }; }
static glm::vec4 JsonVec4(const json& j) { return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>() }; }

// ─── Serialize ────────────────────────────────────────────────────────────────
void SceneSerializer::Serialize(const std::string& filepath) {
    json root;
    root["scene"] = m_Scene->GetName();
    root["entities"] = json::array();

    auto& reg = m_Scene->GetRegistry();
    auto tagView = reg.view<TagComponent>();
    for (auto handle : tagView) {
        Entity entity(handle, m_Scene);
        if (!entity.IsValid()) continue;

        json e;

        if (entity.HasComponent<TagComponent>())
            e["tag"] = entity.GetComponent<TagComponent>().Tag;

        if (entity.HasComponent<TransformComponent>()) {
            auto& tc = entity.GetComponent<TransformComponent>();
            e["transform"] = {
                { "position", Vec2Json(tc.Position) },
                { "scale",    Vec2Json(tc.Scale)    },
                { "rotation", tc.Rotation           }
            };
        }

        if (entity.HasComponent<SpriteRendererComponent>()) {
            auto& sc = entity.GetComponent<SpriteRendererComponent>();
            e["sprite"] = {
                { "color",  Vec4Json(sc.Color) },
                { "zorder", sc.ZOrder          }
            };
        }

        if (entity.HasComponent<CameraComponent>()) {
            auto& cc = entity.GetComponent<CameraComponent>();
            e["camera"] = {
                { "primary",           cc.Primary          },
                { "fixed_aspect",      cc.FixedAspectRatio },
                { "ortho_size",        cc.OrthographicSize },
                { "background_color",  Vec4Json(cc.BackgroundColor) }
            };
        }

        if (entity.HasComponent<RigidBody2DComponent>()) {
            auto& rb = entity.GetComponent<RigidBody2DComponent>();
            e["rigidbody2d"] = {
                { "type",           static_cast<int>(rb.Type) },
                { "fixed_rotation", rb.FixedRotation          }
            };
        }

        if (entity.HasComponent<BoxCollider2DComponent>()) {
            auto& bc = entity.GetComponent<BoxCollider2DComponent>();
            e["box_collider2d"] = {
                { "offset",      Vec2Json(bc.Offset) },
                { "size",        Vec2Json(bc.Size)   },
                { "density",     bc.Density          },
                { "friction",    bc.Friction         },
                { "restitution", bc.Restitution      }
            };
        }

        if (entity.HasComponent<AIComponent>()) {
            auto& ai = entity.GetComponent<AIComponent>();
            e["ai_script"] = {
                { "module", ai.ModuleName },
                { "class",  ai.ClassName  }
            };
        }

        root["entities"].push_back(e);
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        GE_CORE_ERROR("SceneSerializer: cannot open '{}' for writing.", filepath);
        return;
    }
    file << root.dump(2);
    GE_CORE_INFO("Scene '{}' saved → {}", m_Scene->GetName(), filepath);
}

// ─── Deserialize ─────────────────────────────────────────────────────────────
bool SceneSerializer::Deserialize(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        GE_CORE_ERROR("SceneSerializer: cannot open '{}' for reading.", filepath);
        return false;
    }

    json root;
    try { root = json::parse(file); }
    catch (const json::parse_error& e) {
        GE_CORE_ERROR("SceneSerializer: parse error in '{}': {}", filepath, e.what());
        return false;
    }

    for (auto& e : root["entities"]) {
        std::string tag = e.value("tag", "Entity");
        Entity entity = m_Scene->CreateEntity(tag);

        if (e.contains("transform")) {
            auto& j  = e["transform"];
            auto& tc = entity.GetComponent<TransformComponent>();
            tc.Position = JsonVec2(j["position"]);
            tc.Scale    = JsonVec2(j["scale"]);
            tc.Rotation = j["rotation"].get<float>();
        }

        if (e.contains("sprite")) {
            auto& j  = e["sprite"];
            auto& sc = entity.AddComponent<SpriteRendererComponent>();
            sc.Color  = JsonVec4(j["color"]);
            sc.ZOrder = j.value("zorder", 0);
        }

        if (e.contains("camera")) {
            auto& j  = e["camera"];
            auto& cc = entity.AddComponent<CameraComponent>();
            cc.Primary          = j.value("primary",      true);
            cc.FixedAspectRatio = j.value("fixed_aspect", false);
            cc.OrthographicSize = j.value("ortho_size",   7.0f);
            cc.BackgroundColor  = JsonVec4(j["background_color"]);
        }

        if (e.contains("rigidbody2d")) {
            auto& j  = e["rigidbody2d"];
            auto& rb = entity.AddComponent<RigidBody2DComponent>();
            rb.Type          = static_cast<RigidBody2DComponent::BodyType>(j.value("type", 0));
            rb.FixedRotation = j.value("fixed_rotation", false);
        }

        if (e.contains("box_collider2d")) {
            auto& j  = e["box_collider2d"];
            auto& bc = entity.AddComponent<BoxCollider2DComponent>();
            bc.Offset      = JsonVec2(j["offset"]);
            bc.Size        = JsonVec2(j["size"]);
            bc.Density     = j.value("density",     1.0f);
            bc.Friction    = j.value("friction",    0.5f);
            bc.Restitution = j.value("restitution", 0.0f);
        }

        if (e.contains("ai_script")) {
            auto& j  = e["ai_script"];
            entity.AddComponent<AIComponent>(
                j.value("module", ""),
                j.value("class",  "")
            );
        }
    }

    GE_CORE_INFO("Scene loaded ← {}", filepath);
    return true;
}

} // namespace GE
