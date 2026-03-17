#include "GE/Editor/ImGuiLayer.h"
#include "GE/Core/Application.h"
#include "GE/Renderer/Renderer2D.h"
#include "GE/Core/Log.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <SDL2/SDL.h>

namespace GE {

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

void ImGuiLayer::OnAttach() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding  = 4.0f;
    style.FrameRounding   = 3.0f;
    style.PopupRounding   = 3.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding    = 3.0f;
    style.TabRounding     = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FramePadding    = { 6.0f, 4.0f };
    style.ItemSpacing     = { 8.0f, 4.0f };

    auto* nativeWindow   = static_cast<SDL_Window*>(
        Application::Get().GetWindow().GetNativeWindow());
    auto* nativeRenderer = static_cast<SDL_Renderer*>(
        Renderer2D::GetNativeRenderer());

    ImGui_ImplSDL2_InitForSDLRenderer(nativeWindow, nativeRenderer);
    ImGui_ImplSDLRenderer2_Init(nativeRenderer);

    // Load custom font — falls back to ImGui default if file not found
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono.ttf", 14.0f);
    if (!font) {
        io.Fonts->AddFontDefault();
        GE_CORE_WARN("ImGuiLayer: RobotoMono.ttf not found — using default font.");
    }
    ImGui_ImplSDLRenderer2_DestroyFontsTexture();
    ImGui_ImplSDLRenderer2_CreateFontsTexture();

    GE_CORE_INFO("ImGuiLayer attached.");
}

void ImGuiLayer::OnDetach() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::OnEvent(Event& e) {
    if (!m_BlockEvents) return;
    ImGuiIO& io = ImGui::GetIO();
    // If ImGui wants keyboard/mouse, consume the event
    e.Handled |= e.IsInCategory(EventCategory::Keyboard) && io.WantCaptureKeyboard;
    e.Handled |= e.IsInCategory(EventCategory::Mouse)    && io.WantCaptureMouse;
}

void ImGuiLayer::Begin() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End() {
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

} // namespace GE
