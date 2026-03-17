#pragma once

#include "GE/Renderer/RendererAPI.h"
#include <SDL2/SDL.h>

namespace GE {

class SDL2RendererAPI : public RendererAPI {
public:
    explicit SDL2RendererAPI(SDL_Window* window);
    ~SDL2RendererAPI() override;

    void Init()     override;
    void Shutdown() override;

    void SetClearColor(const glm::vec4& color) override;
    void Clear()    override;
    void Present()  override;

    SDL_Renderer* GetSDLRenderer() const { return m_Renderer; }

private:
    SDL_Renderer* m_Renderer = nullptr;
    SDL_Window*   m_Window   = nullptr;
    glm::vec4     m_ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
};

} // namespace GE
