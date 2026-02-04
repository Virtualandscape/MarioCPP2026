#pragma once

#include "mario/world/TileMap.hpp"

namespace mario {

struct TileCollisionResult {
    float x;
    float y;
    float vx;
    float vy;
};

// Swept AABB vs solid tiles; resolves one axis at a time.
TileCollisionResult resolve_tile_collision(float x, float y, float vx, float vy,
                                          float w, float h, const TileMap& map, float dt);

} // namespace mario
