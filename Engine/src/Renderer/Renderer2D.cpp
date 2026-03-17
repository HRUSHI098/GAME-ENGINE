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
    SDL2RendererAPI*  RendererAPI  = nullptr;
    const Camera2D*   ActiveCamera = nullptr;
    Renderer2D::Stats Stats;

    SDL_Texture* ViewportTex = nullptr;
    int          ViewportW   = 0;
    int          ViewportH   = 0;
    int          RenderW     = 0;   // active render target width  (viewport or window)
    int          RenderH     = 0;   // active render target height
};

static Renderer2DData s_Data;

void Renderer2D::Init() {
    auto* nativeWindow = static_cast<SDL_Window*>(
        Application::Get().GetWindow().GetNativeWindow()
    );
    static SDL2RendererAPI api(nativeWindow);
    api.Init();
    s_Data.RendererAPI = &api;
    s_Data.RenderW = static_cast<int>(Application::Get().GetWindow().GetWidth());
    s_Data.RenderH = static_cast<int>(Application::Get().GetWindow().GetHeight());
    GE_CORE_INFO("Renderer2D initialised (SDL2 backend).");
}

void Renderer2D::Shutdown() {
    if (s_Data.ViewportTex) {
        SDL_DestroyTexture(s_Data.ViewportTex);
        s_Data.ViewportTex = nullptr;
    }
    if (s_Data.RendererAPI)
        s_Data.RendererAPI->Shutdown();
}

void Renderer2D::BeginScene(const Camera2D& camera, const glm::vec4& clearColor) {
    s_Data.ActiveCamera = &camera;
    s_Data.RendererAPI->SetClearColor(clearColor);
    s_Data.RendererAPI->Clear();
}

void Renderer2D::EndScene() {
    s_Data.ActiveCamera = nullptr;
}

void Renderer2D::Present() {
    s_Data.RendererAPI->Present();
}

// ─── Viewport render target ───────────────────────────────────────────────────
void Renderer2D::BeginViewport(int w, int h) {
    SDL_Renderer* r = s_Data.RendererAPI->GetSDLRenderer();
    if (w != s_Data.ViewportW || h != s_Data.ViewportH || !s_Data.ViewportTex) {
        if (s_Data.ViewportTex)
            SDL_DestroyTexture(s_Data.ViewportTex);
        s_Data.ViewportTex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                               SDL_TEXTUREACCESS_TARGET, w, h);
        s_Data.ViewportW = w;
        s_Data.ViewportH = h;
    }
    SDL_SetRenderTarget(r, s_Data.ViewportTex);
    s_Data.RenderW = w;
    s_Data.RenderH = h;
}

void Renderer2D::EndViewport() {
    SDL_Renderer* r = s_Data.RendererAPI->GetSDLRenderer();
    SDL_SetRenderTarget(r, nullptr);
    s_Data.RenderW = static_cast<int>(Application::Get().GetWindow().GetWidth());
    s_Data.RenderH = static_cast<int>(Application::Get().GetWindow().GetHeight());
}

void* Renderer2D::GetViewportTexture() {
    return s_Data.ViewportTex;
}

// ─── World-to-screen helpers ─────────────────────────────────────────────────
static SDL_Point WorldToScreen(const glm::vec2& worldPos, int screenW, int screenH,
                               const glm::mat4& vp) {
    glm::vec4 clip = vp * glm::vec4(worldPos, 0.0f, 1.0f);
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
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    SDL_Point tl = WorldToScreen(position,        s_Data.RenderW, s_Data.RenderH, vp);
    SDL_Point br = WorldToScreen(position + size, s_Data.RenderW, s_Data.RenderH, vp);
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
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    SDL_Point tl = WorldToScreen(position,        s_Data.RenderW, s_Data.RenderH, vp);
    SDL_Point br = WorldToScreen(position + size, s_Data.RenderW, s_Data.RenderH, vp);
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
    SDL_Renderer* r = s_Data.RendererAPI->GetSDLRenderer();
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    // Build the 4 corners in world space, rotated around center
    glm::vec2 center = position + size * 0.5f;
    float     hw     = size.x * 0.5f;
    float     hh     = size.y * 0.5f;
    float     cosR   = std::cos(rotationRadians);
    float     sinR   = std::sin(rotationRadians);

    glm::vec2 corners[4] = {
        { -hw, -hh }, {  hw, -hh }, {  hw,  hh }, { -hw,  hh }
    };
    SDL_Vertex verts[4];
    SDL_Color  col = {
        static_cast<u8>(color.r * 255), static_cast<u8>(color.g * 255),
        static_cast<u8>(color.b * 255), static_cast<u8>(color.a * 255)
    };
    for (int i = 0; i < 4; ++i) {
        glm::vec2 rotated = {
            corners[i].x * cosR - corners[i].y * sinR + center.x,
            corners[i].x * sinR + corners[i].y * cosR + center.y
        };
        SDL_Point sp = WorldToScreen(rotated, s_Data.RenderW, s_Data.RenderH, vp);
        verts[i] = { { (float)sp.x, (float)sp.y }, col, { 0.0f, 0.0f } };
    }
    // Two triangles: TL,TR,BR and TL,BR,BL
    int indices[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(r, nullptr, verts, 4, indices, 6);

    ++s_Data.Stats.DrawCalls;
    ++s_Data.Stats.QuadCount;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                 f32 rotationRadians, const Ref<Texture2D>& texture,
                                 const glm::vec4& tint) {
    SDL_Renderer* r   = s_Data.RendererAPI->GetSDLRenderer();
    SDL_Texture*  tex = static_cast<SDL_Texture*>(texture->GetNativeHandle());
    const glm::mat4& vp = s_Data.ActiveCamera->GetViewProjectionMatrix();

    SDL_Point tl = WorldToScreen(position,        s_Data.RenderW, s_Data.RenderH, vp);
    SDL_Point br = WorldToScreen(position + size, s_Data.RenderW, s_Data.RenderH, vp);
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
