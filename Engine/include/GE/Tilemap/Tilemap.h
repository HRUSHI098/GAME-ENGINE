#pragma once
#include "GE/Core/Base.h"
#include "GE/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace GE {

// ─── TilemapComponent ────────────────────────────────────────────────────────
// A 2D grid of tile IDs drawn from a texture atlas.
// Tile ID 0 = empty (not drawn). All other IDs map into the atlas.
struct TilemapComponent {
    Ref<Texture2D> Atlas;       // sprite sheet / tileset image
    glm::vec2  TileSize  = { 16.0f, 16.0f };   // pixels per tile in the atlas
    glm::vec2  TileScale = { 1.0f,  1.0f  };   // world units per tile
    u32        Cols      = 0;   // atlas columns
    u32        Rows      = 0;   // atlas rows

    u32        MapWidth  = 0;
    u32        MapHeight = 0;
    std::vector<int> Tiles;     // row-major, 0 = empty

    void Resize(u32 w, u32 h) {
        MapWidth  = w;
        MapHeight = h;
        Tiles.assign(w * h, 0);
    }

    int  GetTile(u32 x, u32 y) const {
        if (x >= MapWidth || y >= MapHeight) return 0;
        return Tiles[y * MapWidth + x];
    }

    void SetTile(u32 x, u32 y, int id) {
        if (x >= MapWidth || y >= MapHeight) return;
        Tiles[y * MapWidth + x] = id;
    }
};

// ─── TilemapRenderer ─────────────────────────────────────────────────────────
class TilemapRenderer {
public:
    // Call from Scene::OnRender after BeginScene
    static void Render(const TilemapComponent& tm, const glm::vec2& origin);
};

} // namespace GE
