#include <GE/GE.h>
#include <GE/Core/EntryPoint.h>
#include "SandboxLayer.h"

class SandboxApp : public GE::Application {
public:
    SandboxApp()
        : GE::Application(GE::WindowProps{ "GE Sandbox", 1280, 720 }) {
        PushLayer(new SandboxLayer());

#ifdef GE_DEBUG
        PushOverlay(new GE::EditorLayer());
#endif
    }
};

GE::Application* GE::CreateApplication() {
    return new SandboxApp();
}
