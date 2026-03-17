#pragma once

#include "GE/Core/Base.h"
#include <glm/glm.hpp>

namespace GE {

// Abstract 2D renderer interface.
// The SDL2 backend implements this; future OpenGL/Vulkan backends will too.
class RendererAPI {
public:
    enum class API { None = 0, SDL2 = 1, OpenGL = 2, Vulkan = 3 };

    virtual ~RendererAPI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;

    // Present the rendered frame
    virtual void Present() = 0;

    static API GetAPI() { return s_API; }
    static Scope<RendererAPI> Create();

private:
    static API s_API;
};

} // namespace GE
