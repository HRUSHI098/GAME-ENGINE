#pragma once

#include "GE/Core/Base.h"
#include "GE/Renderer/Texture.h"
#include "GE/Renderer/Camera.h"
#include <glm/glm.hpp>

namespace GE {

// High-level 2D rendering API — batches quads/sprites for the active backend.
class Renderer2D {
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(const Camera2D& camera, const glm::vec4& clearColor = { 0.1f, 0.1f, 0.1f, 1.0f });
    static void EndScene();
    static void Present();   // call once per frame after all layers render

    // ─── Primitives ──────────────────────────────────────────────────────────
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color);

    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         const Ref<Texture2D>& texture,
                         const glm::vec4& tint = glm::vec4(1.0f));

    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                f32 rotationRadians, const glm::vec4& color);

    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                f32 rotationRadians, const Ref<Texture2D>& texture,
                                const glm::vec4& tint = glm::vec4(1.0f));

    // ─── Stats ───────────────────────────────────────────────────────────────
    struct Stats {
        u32 DrawCalls = 0;
        u32 QuadCount = 0;
    };
    static void      ResetStats();
    static Stats     GetStats();

    // Returns the underlying SDL_Renderer* as void* (used by ImGui backend)
    static void* GetNativeRenderer();

    // ─── Viewport render target ───────────────────────────────────────────────
    static void BeginViewport(int w, int h);  // redirect draws to offscreen texture
    static void EndViewport();                // restore default render target
    static void* GetViewportTexture();        // returns SDL_Texture*
};

} // namespace GE
