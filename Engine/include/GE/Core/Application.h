#pragma once

#include "GE/Core/Base.h"
#include "GE/Core/Window.h"
#include "GE/Core/LayerStack.h"
#include "GE/Core/Events/Event.h"
#include "GE/Core/Events/WindowEvents.h"

namespace GE {

class Application {
public:
    explicit Application(const WindowProps& props = WindowProps{});
    virtual ~Application();

    void Run();
    void Close();

    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    Window& GetWindow() { return *m_Window; }

    static Application& Get() { return *s_Instance; }

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

    Scope<Window> m_Window;
    LayerStack    m_LayerStack;
    bool          m_Running   = true;
    bool          m_Minimized = false;
    f32           m_LastFrameTime = 0.0f;

    static Application* s_Instance;
};

// Defined by the client (game)
Application* CreateApplication();

} // namespace GE
