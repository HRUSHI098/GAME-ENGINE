#include "GE/Renderer/Texture.h"
#include "GE/Core/Application.h"
#include "GE/Core/Log.h"
#include "GE/Platform/SDL2/SDL2Renderer.h"
#include <SDL2/SDL.h>
#include <SDL_image.h>

namespace GE {

class SDL2Texture2D : public Texture2D {
public:
    SDL2Texture2D(const std::string& path) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            GE_CORE_ERROR("Failed to load texture '{}': {}", path, IMG_GetError());
            return;
        }
        m_Width  = static_cast<u32>(surface->w);
        m_Height = static_cast<u32>(surface->h);

        // Get the SDL renderer from the current backend
        auto* nativeWindow = static_cast<SDL_Window*>(
            Application::Get().GetWindow().GetNativeWindow()
        );
        SDL_Renderer* renderer = SDL_GetRenderer(nativeWindow);
        m_Texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!m_Texture)
            GE_CORE_ERROR("SDL_CreateTextureFromSurface failed: {}", SDL_GetError());
    }

    SDL2Texture2D(u32 width, u32 height, void* data)
        : m_Width(width), m_Height(height) {
        auto* nativeWindow = static_cast<SDL_Window*>(
            Application::Get().GetWindow().GetNativeWindow()
        );
        SDL_Renderer* renderer = SDL_GetRenderer(nativeWindow);
        m_Texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC,
            static_cast<int>(width), static_cast<int>(height));
        if (m_Texture && data)
            SDL_UpdateTexture(m_Texture, nullptr, data, static_cast<int>(width * 4));
    }

    ~SDL2Texture2D() override {
        if (m_Texture) SDL_DestroyTexture(m_Texture);
    }

    u32   GetWidth()        const override { return m_Width; }
    u32   GetHeight()       const override { return m_Height; }
    void* GetNativeHandle() const override { return m_Texture; }

private:
    SDL_Texture* m_Texture = nullptr;
    u32 m_Width  = 0;
    u32 m_Height = 0;
};

// ─── Factory ─────────────────────────────────────────────────────────────────
Ref<Texture2D> Texture2D::Create(const std::string& path) {
    return MakeRef<SDL2Texture2D>(path);
}

Ref<Texture2D> Texture2D::Create(u32 width, u32 height, void* data) {
    return MakeRef<SDL2Texture2D>(width, height, data);
}

} // namespace GE
