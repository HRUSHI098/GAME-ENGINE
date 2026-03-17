#pragma once
#include "GE/Core/Base.h"
#include "GE/Renderer/Texture.h"
#include <glm/glm.hpp>

namespace GE {

// ─── SubTexture2D ─────────────────────────────────────────────────────────────
// Represents a rectangular region of a texture atlas / sprite sheet.
// Coords are in pixels; UV mapping is computed automatically.
class SubTexture2D {
public:
    SubTexture2D(const Ref<Texture2D>& texture,
                 const glm::vec2& pixelMin,   // top-left pixel (inclusive)
                 const glm::vec2& pixelMax);  // bottom-right pixel (exclusive)

    // Convenience factory: spriteCoords is the (col, row) index into a
    // uniform grid of cellSize pixels per cell.
    static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture,
                                              const glm::vec2& spriteCoords,
                                              const glm::vec2& cellSize,
                                              const glm::vec2& spriteSize = { 1.0f, 1.0f });

    const Ref<Texture2D>& GetTexture() const { return m_Texture; }

    // UV coords: [0] = bottom-left, [1] = bottom-right,
    //            [2] = top-right,   [3] = top-left  (SDL convention: flip Y)
    const glm::vec2* GetUVCoords() const { return m_UV; }

private:
    Ref<Texture2D> m_Texture;
    glm::vec2      m_UV[4];
};

} // namespace GE
