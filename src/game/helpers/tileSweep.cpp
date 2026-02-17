// TileSweep.cpp
//
// This file implements collision resolution between a moving object (AABB) and a tile map for a platformer game.
// It uses a swept AABB approach to detect and resolve collisions on each axis separately.
//
// Main functions:
// - rects_intersect: determines if two rectangles intersect.
// - resolve_tile_collision: resolves the collision of an object with the tile map by adjusting its position and velocity.
//
// Details:
// 1. The previous position of the object is calculated based on its velocity and elapsed time.
// 2. Collisions are resolved first on the X axis, then on the Y axis, by checking solid tiles encountered.
// 3. If a collision is detected, the position is adjusted to the edge of the tile and the velocity on that axis is set to zero.
// 4. The function returns the new position and velocity after collision resolution.

#include "mario/game/helpers/tileSweep.hpp"

#include <algorithm>
#include <cmath>

namespace mario {
    namespace {
        // Determines if two rectangles (AABB) intersect.
        inline bool rects_intersect(float ax, float ay, float aw, float ah,
                                    float bx, float by, float bw, float bh) {
            return ax < (bx + bw) && (ax + aw) > bx && ay < (by + bh) && (ay + ah) > by;
        }
    }

    // Resolves the collision of an object with the tile map.
    // x, y: current position; vx, vy: velocity; w, h: size; map: tile map; dt: delta time.
    // Returns the new position and velocity after collision.
    TileCollisionResult resolve_tile_collision(float x, float y, float vx, float vy,
                                               float w, float h, const TileMap& map, float dt) {
        const float prev_x = x - vx * dt; // Previous X position
        const float prev_y = y - vy * dt; // Previous Y position
        const int tile_size = map.tile_size();
        const auto tile_size_f = static_cast<float>(tile_size);

        if (tile_size <= 0) return {x, y, vx, vy};

        float new_x = x;
        float new_y = y;
        float new_vx = vx;
        float new_vy = vy;

        // Resolves collision on one axis (X or Y) at a time.
        // new_pos: position to correct; new_vel: velocity to correct;
        // prev_pos: previous position on this axis; fixed_pos: position on the other axis;
        // size_axis: size on the axis to correct; size_fixed: size on the other axis;
        // horizontal: true for X, false for Y.
        auto resolve_axis = [&](float& new_pos, float& new_vel, float prev_pos, float fixed_pos,
                                float size_axis, float size_fixed, bool horizontal) {
            if (new_vel == 0.0f) return;

            const float min_pos = std::min(prev_pos, new_pos);
            const float max_pos = std::max(prev_pos, new_pos);

            int start_x = 0;
            int end_x = 0;
            int start_y = 0;
            int end_y = 0;

            if (horizontal) {
                start_x = static_cast<int>(std::floor(min_pos / tile_size_f));
                end_x = static_cast<int>(std::floor((max_pos + size_axis) / tile_size_f));
                start_y = static_cast<int>(std::floor(fixed_pos / tile_size_f));
                end_y = static_cast<int>(std::floor((fixed_pos + size_fixed) / tile_size_f));
            } else {
                start_x = static_cast<int>(std::floor(fixed_pos / tile_size_f));
                end_x = static_cast<int>(std::floor((fixed_pos + size_fixed) / tile_size_f));
                start_y = static_cast<int>(std::floor(min_pos / tile_size_f));
                end_y = static_cast<int>(std::floor((max_pos + size_axis) / tile_size_f));
            }

            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) continue;
                    const float tile_left = static_cast<float>(tx) * tile_size_f;
                    const float tile_top = static_cast<float>(ty) * tile_size_f;

                    const float test_x = horizontal ? new_pos : fixed_pos;
                    const float test_y = horizontal ? fixed_pos : new_pos;
                    const float test_w = horizontal ? size_axis : size_fixed;
                    const float test_h = horizontal ? size_fixed : size_axis;

                    if (!rects_intersect(test_x, test_y, test_w, test_h, tile_left, tile_top, tile_size_f, tile_size_f)) continue;

                    // Adjust position to the edge of the tile and set velocity on this axis to zero.
                    if (horizontal) {
                        new_pos = (new_vel > 0.0f) ? tile_left - size_axis : tile_left + tile_size_f;
                    } else {
                        new_pos = (new_vel > 0.0f) ? tile_top - size_axis : tile_top + tile_size_f;
                    }
                    new_vel = 0.0f;
                }
            }
        };

        resolve_axis(new_x, new_vx, prev_x, prev_y, w, h, true);   // X axis
        resolve_axis(new_y, new_vy, prev_y, new_x, h, w, false);   // Y axis

        return {new_x, new_y, new_vx, new_vy};
    }
} // namespace mario
