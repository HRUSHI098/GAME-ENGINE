#include "GE/Renderer/SubTexture2D.h"

namespace GE {

SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture,
                           const glm::vec2& pixelMin,
                           const glm::vec2& pixelMax)
    : m_Texture(texture) {
    float tw = static_cast<float>(texture->GetWidth());
    float th = static_cast<float>(texture->GetHeight());

    float u0 = pixelMin.x / tw,  u1 = pixelMax.x / tw;
    float v0 = pixelMin.y / th,  v1 = pixelMax.y / th;

    m_UV[0] = { u0, v1 };  // bottom-left
    m_UV[1] = { u1, v1 };  // bottom-right
    m_UV[2] = { u1, v0 };  // top-right
    m_UV[3] = { u0, v0 };  // top-left
}

Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture,
                                                  const glm::vec2& coords,
                                                  const glm::vec2& cellSize,
                                                  const glm::vec2& spriteSize) {
    glm::vec2 min = { coords.x * cellSize.x,
                      coords.y * cellSize.y };
    glm::vec2 max = { (coords.x + spriteSize.x) * cellSize.x,
                      (coords.y + spriteSize.y) * cellSize.y };
    return MakeRef<SubTexture2D>(texture, min, max);
}

} // namespace GE
