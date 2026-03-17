#include "SandboxLayer.h"
#include <box2d/box2d.h>

SandboxLayer::SandboxLayer()
    : GE::Layer("Sandbox") {}

void SandboxLayer::OnAttach() {
    m_Scene = MakeScope<GE::Scene>("Physics + Audio Demo");

    // ── Audio assets ──────────────────────────────────────────────────────────
    // Paths are relative to the working directory (build/bin/).
    // Drop WAV/OGG files there, or adjust paths as needed.
    // AudioClip::Create gracefully logs an error if the file is missing.
    m_JumpSound = GE::AudioClip::Create("assets/audio/jump.wav");
    m_LandSound = GE::AudioClip::Create("assets/audio/land.wav");
    m_BGMusic   = GE::MusicTrack::Create("assets/audio/bgmusic.ogg");

    // ── Camera ────────────────────────────────────────────────────────────────
    m_CameraEntity = m_Scene->CreateEntity("Main Camera");
    {
        auto& cc = m_CameraEntity.AddComponent<GE::CameraComponent>();
        cc.Primary = true;
        m_CameraEntity.GetComponent<GE::TransformComponent>().Position = { 0.0f, 2.0f };
        m_CameraEntity.AddComponent<GE::AudioListenerComponent>();
    }

    // ── Static floor ──────────────────────────────────────────────────────────
    {
        auto floor  = m_Scene->CreateEntity("Floor");
        auto& tc    = floor.GetComponent<GE::TransformComponent>();
        tc.Position = { 0.0f, -2.0f };
        tc.Scale    = { 8.0f, 0.4f };
        floor.AddComponent<GE::SpriteRendererComponent>(glm::vec4{ 0.6f, 0.4f, 0.2f, 1.0f });
        auto& rb    = floor.AddComponent<GE::RigidBody2DComponent>();
        rb.Type     = GE::RigidBody2DComponent::BodyType::Static;
        auto& bc    = floor.AddComponent<GE::BoxCollider2DComponent>();
        bc.Friction = 0.6f;
    }

    // ── Dynamic player ────────────────────────────────────────────────────────
    m_PlayerEntity = m_Scene->CreateEntity("Player");
    {
        auto& tc        = m_PlayerEntity.GetComponent<GE::TransformComponent>();
        tc.Position     = { 0.0f, 4.0f };
        tc.Scale        = { 0.5f, 0.5f };
        m_PlayerEntity.AddComponent<GE::SpriteRendererComponent>(
            glm::vec4{ 0.2f, 0.75f, 0.3f, 1.0f });
        auto& rb        = m_PlayerEntity.AddComponent<GE::RigidBody2DComponent>();
        rb.Type         = GE::RigidBody2DComponent::BodyType::Dynamic;
        rb.FixedRotation = true;
        auto& bc        = m_PlayerEntity.AddComponent<GE::BoxCollider2DComponent>();
        bc.Density      = 1.0f;
        bc.Friction     = 0.4f;
        bc.Restitution  = 0.05f;
    }

    // ── Falling boxes ─────────────────────────────────────────────────────────
    const glm::vec4 boxColors[] = {
        { 0.8f, 0.2f, 0.2f, 1.0f },
        { 0.2f, 0.4f, 0.9f, 1.0f },
        { 0.9f, 0.7f, 0.1f, 1.0f },
    };
    for (int i = 0; i < 3; ++i) {
        auto box    = m_Scene->CreateEntity("Box" + std::to_string(i));
        auto& tc    = box.GetComponent<GE::TransformComponent>();
        tc.Position = { -1.5f + i * 1.5f, 6.0f + i * 1.2f };
        tc.Scale    = { 0.4f, 0.4f };
        box.AddComponent<GE::SpriteRendererComponent>(boxColors[i]);
        auto& rb    = box.AddComponent<GE::RigidBody2DComponent>();
        rb.Type     = GE::RigidBody2DComponent::BodyType::Dynamic;
        auto& bc    = box.AddComponent<GE::BoxCollider2DComponent>();
        bc.Density      = 0.8f;
        bc.Restitution  = 0.3f;
    }

    // ── AI entity — Python patrol script ─────────────────────────────────────
    m_AIEntity = m_Scene->CreateEntity("PatrolEnemy");
    {
        auto& tc    = m_AIEntity.GetComponent<GE::TransformComponent>();
        tc.Position = { -1.5f, 4.0f };
        tc.Scale    = { 0.45f, 0.45f };
        m_AIEntity.AddComponent<GE::SpriteRendererComponent>(
            glm::vec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        auto& rb    = m_AIEntity.AddComponent<GE::RigidBody2DComponent>();
        rb.Type     = GE::RigidBody2DComponent::BodyType::Dynamic;
        rb.FixedRotation = true;
        auto& bc    = m_AIEntity.AddComponent<GE::BoxCollider2DComponent>();
        bc.Density  = 1.0f;
        bc.Friction = 0.3f;
        // Attach Python AI — runs patrol_agent.PatrolAgent each frame
        m_AIEntity.AddComponent<GE::AIComponent>("patrol_agent", "PatrolAgent");

        // Also attach a BehaviorTree (mixed C++/Python nodes)
        using BTC = GE::BehaviorTreeComponent;
        std::vector<Scope<GE::BTNode>> children;
        children.push_back(BTC::MakePyLeaf("is_near_boundary"));
        children.push_back(BTC::MakePyLeaf("patrol"));
        auto& btc = m_AIEntity.AddComponent<GE::BehaviorTreeComponent>();
        btc.Root  = BTC::MakeSelector(std::move(children));
    }

    // ── Set viewport so camera projection matches window size ─────────────────
    {
        auto& win = GE::Application::Get().GetWindow();
        m_Scene->OnViewportResize(win.GetWidth(), win.GetHeight());
    }

    // ── Start physics + collision callbacks ───────────────────────────────────
    m_Scene->OnPhysicsStart();

    m_Scene->SetBeginContactCallback([this](const GE::ContactInfo& info) {
        // Play land sound when the player touches anything
        u32 playerID = static_cast<u32>(static_cast<entt::entity>(m_PlayerEntity));
        if (info.EntityA == playerID || info.EntityB == playerID)
            GE::AudioEngine::PlaySound(m_LandSound, 0.7f);
    });

    // Start background music (loops)
    GE::AudioEngine::PlayMusic(m_BGMusic, 0.4f, true);

    GE_INFO("'{}' ready.", m_Scene->GetName());

#ifdef GE_DEBUG
    if (auto* editor = GE::EditorLayer::Get())
        editor->SetScene(m_Scene.get());
#endif
}

void SandboxLayer::OnDetach() {
#ifdef GE_DEBUG
    if (auto* editor = GE::EditorLayer::Get())
        editor->SetScene(nullptr);
#endif
    GE::AudioEngine::StopMusic();
    m_Scene->OnPhysicsStop();
    m_Scene.reset();
}

void SandboxLayer::OnUpdate(f32 dt) {
    // ── Camera movement ───────────────────────────────────────────────────────
    auto& camT = m_CameraEntity.GetComponent<GE::TransformComponent>();
    if (GE::Input::IsKeyPressed(GE::KeyCode::A)) camT.Position.x -= m_CameraSpeed * dt;
    if (GE::Input::IsKeyPressed(GE::KeyCode::D)) camT.Position.x += m_CameraSpeed * dt;
    if (GE::Input::IsKeyPressed(GE::KeyCode::W)) camT.Position.y += m_CameraSpeed * dt;
    if (GE::Input::IsKeyPressed(GE::KeyCode::S)) camT.Position.y -= m_CameraSpeed * dt;

    // ── Player physics ────────────────────────────────────────────────────────
    auto& rb = m_PlayerEntity.GetComponent<GE::RigidBody2DComponent>();
    if (rb.RuntimeBody) {
        b2Vec2 vel  = rb.RuntimeBody->GetLinearVelocity();
        bool grounded = std::abs(vel.y) < 0.05f;

        const f32 targetVx =
            GE::Input::IsKeyPressed(GE::KeyCode::Left)  ? -4.0f :
            GE::Input::IsKeyPressed(GE::KeyCode::Right) ?  4.0f : 0.0f;
        rb.RuntimeBody->SetLinearVelocity({ targetVx, vel.y });

        if (GE::Input::IsKeyPressed(GE::KeyCode::Up) && grounded) {
            rb.RuntimeBody->ApplyLinearImpulseToCenter({ 0.0f, 6.0f }, true);
            GE::AudioEngine::PlaySound(m_JumpSound, 0.9f);
        }
    }

    // ── Music toggle (M key) ──────────────────────────────────────────────────
    static bool mWasPressed = false;
    bool mPressed = GE::Input::IsKeyPressed(GE::KeyCode::M);
    if (mPressed && !mWasPressed) {
        if (GE::AudioEngine::IsMusicPaused())
            GE::AudioEngine::ResumeMusic();
        else
            GE::AudioEngine::PauseMusic();
    }
    mWasPressed = mPressed;

    m_Scene->OnUpdate(dt);
    m_Scene->OnRender();
}

void SandboxLayer::OnEvent(GE::Event& e) {
    GE::EventDispatcher d(e);
    d.Dispatch<GE::WindowResizeEvent>([this](GE::WindowResizeEvent& ev) {
        m_Scene->OnViewportResize(ev.GetWidth(), ev.GetHeight());
        return false;
    });
}
