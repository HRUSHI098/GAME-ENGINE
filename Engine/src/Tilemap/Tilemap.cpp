#include "GE/Tilemap/Tilemap.h"
#include "GE/Renderer/Renderer2D.h"
#include "GE/Renderer/SubTexture2D.h"

namespace GE {

void TilemapRenderer::Render(const TilemapComponent& tm, const glm::vec2& origin) {
    if (!tm.Atlas || tm.Cols == 0 || tm.Rows == 0) return;

    for (u32 y = 0; y < tm.MapHeight; ++y) {
        for (u32 x = 0; x < tm.MapWidth; ++x) {
            int id = tm.GetTile(x, y);
            if (id <= 0) continue;

            int tileIdx = id - 1;  // 1-based → 0-based
            u32 col = static_cast<u32>(tileIdx) % tm.Cols;
            u32 row = static_cast<u32>(tileIdx) / tm.Cols;

            auto sub = SubTexture2D::CreateFromCoords(
                tm.Atlas,
                { static_cast<float>(col), static_cast<float>(row) },
                tm.TileSize
            );

            glm::vec2 pos = origin + glm::vec2(x, y) * tm.TileScale;
            Renderer2D::DrawSubTexture(pos, tm.TileScale, sub);
        }
    }
}

} // namespace GE
