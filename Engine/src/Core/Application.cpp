#include "GE/Core/Application.h"
#include "GE/Core/Log.h"
#include "GE/Renderer/Renderer2D.h"
#include "GE/Audio/AudioEngine.h"
#include <SDL2/SDL.h>

namespace GE {

Application* Application::s_Instance = nullptr;

Application::Application(const WindowProps& props) {
    GE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    m_Window = Window::Create(props);
    m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
    Renderer2D::Init();
    AudioEngine::Init();
}

Application::~Application() {
    SDL_Quit();
}

void Application::PushLayer(Layer* layer) {
    m_LayerStack.PushLayer(layer);
    layer->OnAttach();
}

void Application::PushOverlay(Layer* overlay) {
    m_LayerStack.PushOverlay(overlay);
    overlay->OnAttach();
}

void Application::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& ev) {
        return OnWindowClose(ev);
    });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& ev) {
        return OnWindowResize(ev);
    });

    // Propagate to layers (top-most overlay first)
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        if (e.Handled) break;
        (*it)->OnEvent(e);
    }
}

void Application::Run() {
    GE_CORE_INFO("Engine running.");

    while (m_Running) {
        f32 time = static_cast<f32>(SDL_GetTicks()) / 1000.0f;
        f32 deltaTime = time - m_LastFrameTime;
        m_LastFrameTime = time;

        m_Window->OnUpdate();   // polls events + swaps

        if (!m_Minimized) {
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(deltaTime);
            Renderer2D::Present();
        }
    }
}

void Application::Close() {
    m_Running = false;
}

bool Application::OnWindowClose(WindowCloseEvent&) {
    m_Running = false;
    return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e) {
    if (e.GetWidth() == 0 || e.GetHeight() == 0) {
        m_Minimized = true;
        return false;
    }
    m_Minimized = false;
    return false;
}

} // namespace GE
