#include "GE/Renderer/Renderer2D.h"
#include "GE/Core/Application.h"
#include "GE/Core/Log.h"
#include "GE/Platform/SDL2/SDL2Renderer.h"
#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace GE {

// ─── Internal state ───────────────────────────────────────────────────────────
struct Renderer2DData {
    SDL2RendererAPI* RendererAPI = nullptr;
    const Camera2D*  ActiveCamera = nullptr;
    Renderer2D::Stats Stats;
};

static Renderer2DData s_Data;

void Renderer2D::Init() {
    auto* nativeWindow = static_cast<SDL_Window*>(
        Application::Get().GetWindow().GetNativeWindow()
    );
    // The SDL2RendererAPI is owned here for the lifetime of the renderer.
    static SDL2RendererAPI api(nativeWindow);
    api.Init();
    s_Data.RendererAPI = &api;
    GE_CORE_INFO("Renderer2D initialised (SDL2 backend).");
}

void Renderer2D::Shutdown() {
    if (s_Data.RendererAPI)
        s_Data.RendererAPI->Shutdown();
}

void Renderer2D::BeginScene(const Camera2D& camera) {
    s_Data.ActiveCamera = &camera;
    s_Data.RendererAPI->Clear();
}

void Renderer2D::EndScene() {
    s_Data.ActiveCamera = nullptr;
}

void Renderer2D::Present() {
    s_Data.RendererAPI->Present();
}

// ─── World-to-screen helpers ─────────────────────────────────────────────────
// Transform a world-space point through the camera's view-projection matrix
// and map to SDL screen pixels.
static SDL_Point WorldToScreen(const glm::vec2& worldPos, u32 screenW, u32 screenH,
                               const glm::mat4& vp) {
    glm::vec4 clip = vp * glm::vec4(worldPos, 0.0f, 1.0f);
    // NDC → screen
    float ndcX = clip.x / clip.w;
    float ndcY = clip.y / clip.w;
    int sx = static_cast<int>((ndcX + 1.0f) * 0.5f * screenW);
    int sy = static_cast<int>((1.0f - ndcY) * 0.5f * screenH);
    return { sx, sy };
}

// ─── Draw calls ──────────────────────────────────────────────────────────────
void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color) {
    SDL_Renderer* r = s_Data.RendererAPI->GetSDLRenderer();
    u32 w = Application::Get().GetWindow().GetWidth();
    u32 h = Application::Get().GetWindow().GetHeight();
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    // Transform top-left and bottom-right corners
    SDL_Point tl = WorldToScreen(position,                         w, h, vp);
    SDL_Point br = WorldToScreen(position + size,                  w, h, vp);

    SDL_Rect rect = { tl.x, tl.y, br.x - tl.x, br.y - tl.y };

    SDL_SetRenderDrawColor(r,
        static_cast<u8>(color.r * 255),
        static_cast<u8>(color.g * 255),
        static_cast<u8>(color.b * 255),
        static_cast<u8>(color.a * 255));
    SDL_RenderFillRect(r, &rect);

    ++s_Data.Stats.DrawCalls;
    ++s_Data.Stats.QuadCount;
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const Ref<Texture2D>& texture, const glm::vec4& tint) {
    SDL_Renderer* r   = s_Data.RendererAPI->GetSDLRenderer();
    SDL_Texture*  tex = static_cast<SDL_Texture*>(texture->GetNativeHandle());
    u32 w = Application::Get().GetWindow().GetWidth();
    u32 h = Application::Get().GetWindow().GetHeight();
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    SDL_Point tl = WorldToScreen(position,        w, h, vp);
    SDL_Point br = WorldToScreen(position + size, w, h, vp);

    SDL_Rect dst = { tl.x, tl.y, br.x - tl.x, br.y - tl.y };

    SDL_SetTextureColorMod(tex,
        static_cast<u8>(tint.r * 255),
        static_cast<u8>(tint.g * 255),
        static_cast<u8>(tint.b * 255));
    SDL_SetTextureAlphaMod(tex, static_cast<u8>(tint.a * 255));
    SDL_RenderCopy(r, tex, nullptr, &dst);

    ++s_Data.Stats.DrawCalls;
    ++s_Data.Stats.QuadCount;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                 f32 rotationRadians, const glm::vec4& color) {
    // For rotated quads, render to a texture then SDL_RenderCopyEx.
    // For now, fall back to axis-aligned — rotation support extended in Phase 2.
    (void)rotationRadians;
    DrawQuad(position, size, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                 f32 rotationRadians, const Ref<Texture2D>& texture,
                                 const glm::vec4& tint) {
    SDL_Renderer* r   = s_Data.RendererAPI->GetSDLRenderer();
    SDL_Texture*  tex = static_cast<SDL_Texture*>(texture->GetNativeHandle());
    u32 w = Application::Get().GetWindow().GetWidth();
    u32 h = Application::Get().GetWindow().GetHeight();
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    SDL_Point tl = WorldToScreen(position,        w, h, vp);
    SDL_Point br = WorldToScreen(position + size, w, h, vp);

    SDL_Rect dst = { tl.x, tl.y, br.x - tl.x, br.y - tl.y };
    SDL_Point center = { dst.w / 2, dst.h / 2 };
    double angleDeg  = static_cast<double>(rotationRadians) * (180.0 / M_PI);

    SDL_SetTextureColorMod(tex,
        static_cast<u8>(tint.r * 255),
        static_cast<u8>(tint.g * 255),
        static_cast<u8>(tint.b * 255));
    SDL_SetTextureAlphaMod(tex, static_cast<u8>(tint.a * 255));
    SDL_RenderCopyEx(r, tex, nullptr, &dst, angleDeg, &center, SDL_FLIP_NONE);

    ++s_Data.Stats.DrawCalls;
    ++s_Data.Stats.QuadCount;
}

void Renderer2D::ResetStats() {
    s_Data.Stats = {};
}

Renderer2D::Stats Renderer2D::GetStats() {
    return s_Data.Stats;
}

void* Renderer2D::GetNativeRenderer() {
    return s_Data.RendererAPI ? s_Data.RendererAPI->GetSDLRenderer() : nullptr;
}

} // namespace GE
