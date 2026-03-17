#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"
#include "GE/ECS/Components.h"
#include "GE/Physics/PhysicsComponents.h"
#include "GE/Audio/AudioComponents.h"
#include "GE/Audio/AudioEngine.h"
#ifdef GE_SCRIPTING_ENABLED
#include "GE/Scripting/ScriptEngine.h"
#include "GE/Scripting/ScriptComponent.h"
#endif
#include "GE/AI/AISystem.h"
#include "GE/AI/AIComponent.h"
#include "GE/AI/BehaviorTree.h"
#include "GE/AI/MLEnvironment.h"
#include "GE/Renderer/Renderer2D.h"
#include "GE/Core/Log.h"

#include <box2d/box2d.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace GE {

// ─── Entity::GetRegistry() — defined here to break the circular include ───────
entt::registry& Entity::GetRegistry() const {
    return m_Scene->GetRegistry();
}

// ─── Helpers ─────────────────────────────────────────────────────────────────
static b2BodyType ToBox2DBodyType(RigidBody2DComponent::BodyType t) {
    switch (t) {
        case RigidBody2DComponent::BodyType::Static:    return b2_staticBody;
        case RigidBody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
        case RigidBody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
    }
    return b2_staticBody;
}

// ─── Scene ────────────────────────────────────────────────────────────────────
Scene::Scene(std::string name)
    : m_Name(std::move(name)) {}

Scene::~Scene() {
    OnPhysicsStop();

    m_Registry.view<NativeScriptComponent>().each([](auto& nsc) {
        if (nsc.Instance) {
            nsc.Instance->OnDestroy();
            nsc.DestroyFn();
        }
    });
}

Entity Scene::CreateEntity(const std::string& name) {
    Entity entity(m_Registry.create(), this);
    entity.AddComponent<TagComponent>(name);
    entity.AddComponent<TransformComponent>();
    return entity;
}

void Scene::DestroyEntity(Entity entity) {
    // If the physics world is running, destroy the b2Body first
    if (m_Physics && entity.HasComponent<RigidBody2DComponent>()) {
        auto& rb = entity.GetComponent<RigidBody2DComponent>();
        if (rb.RuntimeBody)
            m_Physics->GetWorld()->DestroyBody(rb.RuntimeBody);
    }
    m_Registry.destroy(entity);
}

// ─── Physics lifecycle ────────────────────────────────────────────────────────
void Scene::OnPhysicsStart() {
    m_Physics = MakeScope<Physics2D>();

    // Create b2Bodies for every entity that has a RigidBody2DComponent
    auto view = m_Registry.view<RigidBody2DComponent, TransformComponent>();
    for (auto e : view) {
        auto& tc = view.get<TransformComponent>(e);
        auto& rb = view.get<RigidBody2DComponent>(e);

        b2BodyDef bodyDef;
        bodyDef.type     = ToBox2DBodyType(rb.Type);
        bodyDef.position = { tc.Position.x, tc.Position.y };
        bodyDef.angle    = tc.Rotation;
        bodyDef.fixedRotation = rb.FixedRotation;
        // Store the entt handle so the ContactListener can recover it
        bodyDef.userData.pointer = static_cast<uintptr_t>(static_cast<u32>(e));

        rb.RuntimeBody = m_Physics->GetWorld()->CreateBody(&bodyDef);

        // ── BoxCollider ───────────────────────────────────────────────────────
        if (m_Registry.all_of<BoxCollider2DComponent>(e)) {
            auto& bc = m_Registry.get<BoxCollider2DComponent>(e);

            b2PolygonShape shape;
            shape.SetAsBox(bc.Size.x * tc.Scale.x,
                           bc.Size.y * tc.Scale.y,
                           { bc.Offset.x, bc.Offset.y }, 0.0f);

            b2FixtureDef fixDef;
            fixDef.shape                = &shape;
            fixDef.density              = bc.Density;
            fixDef.friction             = bc.Friction;
            fixDef.restitution          = bc.Restitution;
            fixDef.restitutionThreshold = bc.RestitutionThreshold;

            bc.RuntimeFixture = rb.RuntimeBody->CreateFixture(&fixDef);
        }

        // ── CircleCollider ────────────────────────────────────────────────────
        if (m_Registry.all_of<CircleCollider2DComponent>(e)) {
            auto& cc = m_Registry.get<CircleCollider2DComponent>(e);

            b2CircleShape shape;
            shape.m_p.Set(cc.Offset.x, cc.Offset.y);
            shape.m_radius = cc.Radius * glm::max(tc.Scale.x, tc.Scale.y);

            b2FixtureDef fixDef;
            fixDef.shape                = &shape;
            fixDef.density              = cc.Density;
            fixDef.friction             = cc.Friction;
            fixDef.restitution          = cc.Restitution;
            fixDef.restitutionThreshold = cc.RestitutionThreshold;

            cc.RuntimeFixture = rb.RuntimeBody->CreateFixture(&fixDef);
        }
    }

    GE_CORE_INFO("Physics world started ({} bodies).", view.size_hint());

    // ── Start C# script instances ─────────────────────────────────────────────
#ifdef GE_SCRIPTING_ENABLED
    ScriptEngine::OnSceneStart(this);
#endif

    // ── Start Python AI scripts ───────────────────────────────────────────────
    AISystem_OnSceneStart(this);

    // ── PlayOnAwake audio sources ─────────────────────────────────────────────
    m_Registry.view<AudioSourceComponent>().each([](auto& asc) {
        if (asc.PlayOnAwake && asc.Clip)
            asc.Channel = AudioEngine::PlaySound(asc.Clip, asc.Volume, asc.Loop);
    });
}

void Scene::OnPhysicsStop() {
    // ── Stop Python AI ───────────────────────────────────────────────────────
    AISystem_OnSceneStop(this);

    // ── Stop C# scripts ───────────────────────────────────────────────────────
#ifdef GE_SCRIPTING_ENABLED
    ScriptEngine::OnSceneStop();
#endif

    // Stop all entity-owned audio channels
    m_Registry.view<AudioSourceComponent>().each([](auto& asc) {
        if (asc.Channel >= 0) {
            AudioEngine::StopSound(asc.Channel);
            asc.Channel = -1;
        }
    });

    m_Physics.reset();
    // Clear runtime pointers — they're dangling after world destruction
    m_Registry.view<RigidBody2DComponent>().each([](auto& rb) {
        rb.RuntimeBody = nullptr;
    });
    m_Registry.view<BoxCollider2DComponent>().each([](auto& bc) {
        bc.RuntimeFixture = nullptr;
    });
    m_Registry.view<CircleCollider2DComponent>().each([](auto& cc) {
        cc.RuntimeFixture = nullptr;
    });
}

void Scene::SetGravity(const glm::vec2& gravity) {
    if (m_Physics) m_Physics->SetGravity(gravity);
}

void Scene::SetBeginContactCallback(Physics2D::ContactCallback cb) {
    if (m_Physics) m_Physics->SetBeginContactCallback(std::move(cb));
}

void Scene::SetEndContactCallback(Physics2D::ContactCallback cb) {
    if (m_Physics) m_Physics->SetEndContactCallback(std::move(cb));
}

// ─── Per-frame ────────────────────────────────────────────────────────────────
void Scene::OnViewportResize(u32 width, u32 height) {
    m_ViewportWidth  = width;
    m_ViewportHeight = height;

    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view) {
        auto& cc = view.get<CameraComponent>(entity);
        if (!cc.FixedAspectRatio) {
            f32 aspect = static_cast<f32>(width) / static_cast<f32>(height);
            f32 h = cc.OrthographicSize;
            cc.Camera.SetProjection(-aspect * h, aspect * h, -h, h);
        }
    }
}

void Scene::OnUpdate(f32 dt) {
    // ── C# scripts ───────────────────────────────────────────────────────────
#ifdef GE_SCRIPTING_ENABLED
    m_Registry.view<ScriptComponent>().each([&](auto entity, ScriptComponent& sc) {
        ScriptEngine::OnUpdate(Entity(entity, this), dt);
    });
#endif

    // ── Python AI + Behavior Trees + ML environments ──────────────────────────
    AISystem_OnUpdate(this, dt);

    // ── Native scripts ────────────────────────────────────────────────────────
    m_Registry.view<NativeScriptComponent>().each([dt](auto entity, auto& nsc) {
        if (!nsc.Instance) {
            nsc.Instance = nsc.InstantiateFn();
            nsc.Instance->OnCreate();
        }
        nsc.Instance->OnUpdate(dt);
    });

    // ── Physics step ─────────────────────────────────────────────────────────
    if (m_Physics) {
        m_Physics->Step(dt);

        // Write b2Body results back into TransformComponents
        auto view = m_Registry.view<RigidBody2DComponent, TransformComponent>();
        for (auto e : view) {
            auto& tc = view.get<TransformComponent>(e);
            auto& rb = view.get<RigidBody2DComponent>(e);

            if (!rb.RuntimeBody) continue;
            const b2Vec2& pos = rb.RuntimeBody->GetPosition();
            tc.Position.x = pos.x;
            tc.Position.y = pos.y;
            tc.Rotation   = rb.RuntimeBody->GetAngle();
        }
    }
}

void Scene::OnRender() {
    // ── Find primary camera ───────────────────────────────────────────────────
    Camera2D* mainCamera = nullptr;
    {
        auto view = m_Registry.view<CameraComponent, TransformComponent>();
        for (auto entity : view) {
            auto& cc = view.get<CameraComponent>(entity);
            auto& tc = view.get<TransformComponent>(entity);
            if (cc.Primary) {
                cc.Camera.SetPosition({ tc.Position.x, tc.Position.y, 0.0f });
                cc.Camera.SetRotation(glm::degrees(tc.Rotation));
                mainCamera = &cc.Camera;
                break;
            }
        }
    }
    if (!mainCamera) return;

    // ── Render sprites sorted by ZOrder ──────────────────────────────────────
    Renderer2D::BeginScene(*mainCamera);

    struct RenderEntry {
        TransformComponent*      Transform;
        SpriteRendererComponent* Sprite;
    };
    std::vector<RenderEntry> entries;

    auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
    for (auto entity : view) {
        auto& tc = view.get<TransformComponent>(entity);
        auto& sc = view.get<SpriteRendererComponent>(entity);
        entries.push_back({ &tc, &sc });
    }
    std::sort(entries.begin(), entries.end(), [](const RenderEntry& a, const RenderEntry& b) {
        return a.Sprite->ZOrder < b.Sprite->ZOrder;
    });

    for (auto& entry : entries) {
        auto& tc = *entry.Transform;
        auto& sc = *entry.Sprite;
        if (sc.Texture)
            Renderer2D::DrawRotatedQuad(tc.Position, tc.Scale, tc.Rotation, sc.Texture, sc.Color);
        else
            Renderer2D::DrawRotatedQuad(tc.Position, tc.Scale, tc.Rotation, sc.Color);
    }

    Renderer2D::EndScene();
}

Entity Scene::FindByName(const std::string& name) {
    auto view = m_Registry.view<TagComponent>();
    for (auto entity : view)
        if (view.get<TagComponent>(entity).Tag == name)
            return { entity, this };
    return {};
}

} // namespace GE
