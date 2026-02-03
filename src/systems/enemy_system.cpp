#include <cmath>
#include <algorithm>
#include "mario/systems/EnemySystem.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/CollisionInfoComponent.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    void EnemySystem::update(EntityManager& registry, const TileMap& map, float dt) const {
        auto entities = registry.get_entities_with<EnemyComponent>();
        for (auto entity : entities) {
            auto* enemy = registry.get_component<EnemyComponent>(entity);
            auto* vel = registry.get_component<VelocityComponent>(entity);
            auto* coll = registry.get_component<CollisionInfoComponent>(entity);
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);

            if (enemy && vel && coll && pos && size) {
                // Do NOT inject or force a default speed here. The spawner is the single
                // place that should set enemy initial speeds. Only react to collisions
                // and platform edges by reversing the current horizontal velocity when
                // it has a meaningful magnitude.
                const float MIN_SPEED_THRES = 0.1f;
                float speed = std::abs(vel->vx);

                // If collided horizontally with something, reverse direction only if
                // there is a meaningful horizontal velocity to invert. If velocity is
                // near zero, leave it as-is so the spawner's chosen value (including 0)
                // is preserved.
                if (coll->collided && speed >= MIN_SPEED_THRES) {
                    vel->vx = -vel->vx;
                    speed = std::abs(vel->vx);
                }

                // Do not normalize or force a default horizontal velocity here.

                // Constrain movement to the contiguous solid platform beneath the enemy.
                const int tile_size = map.tile_size();
                if (tile_size <= 0) continue;

                // Calculate tile coordinates for the tile directly below the enemy's feet.
                const float feet_x = pos->x + size->width * 0.5f; // center x
                const float feet_y = pos->y + size->height; // bottom y

                const int tile_x = static_cast<int>(std::floor(feet_x / static_cast<float>(tile_size)));
                const int tile_y = static_cast<int>(std::floor((feet_y + 1.0f) / static_cast<float>(tile_size)));

                // If there's no solid tile directly below, don't constrain (falling or platform edge)
                if (!map.is_solid(tile_x, tile_y)) {
                    continue;
                }

                // Find leftmost and rightmost continuous solid tiles for this platform row
                int left_tx = tile_x;
                while (left_tx - 1 >= 0 && map.is_solid(left_tx - 1, tile_y)) --left_tx;

                int right_tx = tile_x;
                while (right_tx + 1 < map.width() && map.is_solid(right_tx + 1, tile_y)) ++right_tx;

                // Convert tile bounds to world coordinates (tile extents)
                const auto platform_left = static_cast<float>(left_tx * tile_size);
                const auto platform_right = static_cast<float>((right_tx + 1) * tile_size); // exclusive right edge

                // If enemy reaches or passes an edge, reverse direction only if current
                // horizontal velocity is meaningful; do not set a fallback magnitude.
                const float next_x = pos->x + vel->vx * dt;

                const float eps = 0.001f;
                if (vel->vx > 0.0f && (next_x + size->width) > (platform_right - eps)) {
                    if (std::abs(vel->vx) >= MIN_SPEED_THRES) vel->vx = -vel->vx;
                } else if (vel->vx < 0.0f && next_x < (platform_left + eps)) {
                    if (std::abs(vel->vx) >= MIN_SPEED_THRES) vel->vx = -vel->vx;
                }

                // Let physics/collision systems handle final position clamping to avoid jitter.
            }
        }
    }
} // namespace mario
