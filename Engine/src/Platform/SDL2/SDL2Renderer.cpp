#include "GE/Platform/SDL2/SDL2Renderer.h"
#include "GE/Core/Log.h"

namespace GE {

// ─── Factory ─────────────────────────────────────────────────────────────────
RendererAPI::API RendererAPI::s_API = RendererAPI::API::SDL2;

Scope<RendererAPI> RendererAPI::Create() {
    // Window must already be created; obtain it via Application::Get().GetWindow().
    // The Renderer2D init path passes the native handle.
    GE_CORE_WARN("RendererAPI::Create() called without window — use SDL2RendererAPI(window) directly.");
    return nullptr;
}

// ─── SDL2RendererAPI ─────────────────────────────────────────────────────────
SDL2RendererAPI::SDL2RendererAPI(SDL_Window* window)
    : m_Window(window) {}

SDL2RendererAPI::~SDL2RendererAPI() {
    Shutdown();
}

void SDL2RendererAPI::Init() {
    u32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    m_Renderer = SDL_CreateRenderer(m_Window, -1, flags);
    if (!m_Renderer) {
        GE_CORE_FATAL("SDL_CreateRenderer failed: {}", SDL_GetError());
        return;
    }
    SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);
    GE_CORE_INFO("SDL2 renderer initialised.");
}

void SDL2RendererAPI::Shutdown() {
    if (m_Renderer) {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
}

void SDL2RendererAPI::SetClearColor(const glm::vec4& color) {
    m_ClearColor = color;
}

void SDL2RendererAPI::Clear() {
    SDL_SetRenderDrawColor(
        m_Renderer,
        static_cast<u8>(m_ClearColor.r * 255),
        static_cast<u8>(m_ClearColor.g * 255),
        static_cast<u8>(m_ClearColor.b * 255),
        static_cast<u8>(m_ClearColor.a * 255)
    );
    SDL_RenderClear(m_Renderer);
}

void SDL2RendererAPI::Present() {
    SDL_RenderPresent(m_Renderer);
}

} // namespace GE
