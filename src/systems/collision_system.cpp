#include <algorithm>
#include <cmath>
#include "mario/systems/CollisionSystem.hpp"
#include "mario/entities/Entity.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    namespace {
        struct Aabb {
            float left;
            float top;
            float right;
            float bottom;
        };

        // Make an axis-aligned bounding box (AABB) from position and dimensions
        Aabb make_aabb(float x, float y, float w, float h) {
            return {x, y, x + w, y + h};
        }

        // Check if two axis-aligned bounding boxes (AABBs) intersect
        bool intersects(const Aabb &a, const Aabb &b) {
            return a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top;
        }
    }

    // Check if the entity collides with any tiles in the tile map
    void CollisionSystem::check_entity_collision(Entity &entity, const TileMap &map, float dt) const {
        // Clear previous collision info
        entity.clear_collision_info();

        const float w = entity.width();
        const float h = entity.height();
        const float current_x = entity.x();
        const float current_y = entity.y();
        const float prev_x = current_x - entity.vx() * dt;
        const float prev_y = current_y - entity.vy() * dt;
        const int tile_size = map.tile_size();

        if (tile_size <= 0) {
            return;
        }

        float new_x = current_x;
        float new_y = current_y;
        float vx = entity.vx();
        float vy = entity.vy();

        // On the x-axis
        if (vx != 0.0f) {
            const float min_x = std::min(prev_x, current_x);
            const float max_x = std::max(prev_x, current_x);
            const int start_x = static_cast<int>(std::floor(min_x / tile_size));
            const int end_x = static_cast<int>(std::floor((max_x + w) / tile_size));
            const int start_y = static_cast<int>(std::floor(prev_y / tile_size));
            const int end_y = static_cast<int>(std::floor((prev_y + h) / tile_size));

            // Iterate over tiles that the entity might collide with
            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) {
                        continue;
                    }
                    // Build collision detection for the entity with the tile
                    const float tile_left = static_cast<float>(tx * tile_size);
                    const float tile_top = static_cast<float>(ty * tile_size);
                    const Aabb tile_box = {tile_left, tile_top, tile_left + tile_size, tile_top + tile_size};
                    const Aabb entity_box = make_aabb(new_x, prev_y, w, h);
                    if (!intersects(entity_box, tile_box)) {
                        continue;
                    }

                    // If the entity is moving right, stop at the left edge of the tile
                    if (vx > 0.0f) {
                        new_x = tile_left - w;
                    // If the entity is moving left, stop at the right edge of the tile
                    } else {
                        new_x = tile_left + tile_size;
                    }
                    vx = 0.0f;
                }
            }
        }
        // On the y-axis
        if (vy != 0.0f) {
            const float min_y = std::min(prev_y, current_y);
            const float max_y = std::max(prev_y, current_y);
            const int start_x = static_cast<int>(std::floor(new_x / tile_size));
            const int end_x = static_cast<int>(std::floor((new_x + w) / tile_size));
            const int start_y = static_cast<int>(std::floor(min_y / tile_size));
            const int end_y = static_cast<int>(std::floor((max_y + h) / tile_size));

            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) {
                        continue;
                    }

                    const float tile_left = static_cast<float>(tx * tile_size);
                    const float tile_top = static_cast<float>(ty * tile_size);
                    const Aabb tile_box = {tile_left, tile_top, tile_left + tile_size, tile_top + tile_size};
                    const Aabb entity_box = make_aabb(new_x, new_y, w, h);
                    if (!intersects(entity_box, tile_box)) {
                        continue;
                    }

                    if (vy > 0.0f) {
                        new_y = tile_top - h;
                    } else {
                        new_y = tile_top + tile_size;
                    }
                    vy = 0.0f;
                }
            }
        }

        entity.set_position(new_x, new_y);
        entity.set_velocity(vx, vy);
    }

    void CollisionSystem::check_entity_vs_entity_collision(Entity &a, Entity &b) const {
        const Aabb box_a = make_aabb(a.x(), a.y(), a.width(), a.height());
        const Aabb box_b = make_aabb(b.x(), b.y(), b.width(), b.height());

        if (intersects(box_a, box_b)) {
        a.set_collision_info({true, b.type()});
        b.set_collision_info({true, a.type()});

        if (a.type() == EntityType::Player) {
            // Calculate intersection depths
            float overlapLeft = box_b.right - box_a.left;
            float overlapRight = box_a.right - box_b.left;
            float overlapTop = box_b.bottom - box_a.top;
            float overlapBottom = box_a.bottom - box_b.top;

            // Find the smallest overlap on x and y axes
            float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
            float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

            // Resolve along the axis of least penetration
            if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                // Move player horizontally out of collision
                a.set_position(a.x() + minOverlapX, a.y());
                // Stop horizontal velocity
                a.set_velocity(0.0f, a.vy());
            } else {
                // Move the player vertically out of collision
                a.set_position(a.x(), a.y() + minOverlapY);
                // Stop vertical velocity
                a.set_velocity(a.vx(), 0.0f);
            }
        }
    }
}

    void Collider::set_solid(bool solid) { (void) solid; }

    void Hitbox::set_size(float w, float h) {
        (void) w;
        (void) h;
    }
} // namespace mario
