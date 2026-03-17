#pragma once

#include <GE/GE.h>

class SandboxLayer : public GE::Layer {
public:
    SandboxLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(f32 deltaTime) override;
    void OnEvent(GE::Event& e) override;

private:
    Scope<GE::Scene> m_Scene;

    GE::Entity m_CameraEntity;
    GE::Entity m_PlayerEntity;

    f32 m_CameraSpeed = 4.0f;

    // Audio assets
    Ref<GE::AudioClip>   m_JumpSound;
    Ref<GE::AudioClip>   m_LandSound;
    Ref<GE::MusicTrack>  m_BGMusic;

    bool m_WasGrounded = false;
    GE::Entity m_AIEntity;
};
