// Implements the EnemySystem, which updates enemy movement and handles collision response for enemy entities.
// Reverses enemy direction on collision and constrains movement to contiguous platforms (ECS best practice: data-driven logic).

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
    // Updates all enemy entities: reverses direction on collision and constrains movement to platform bounds.
    // Follows ECS principle: systems operate on components, not entity types (though we use EnemyComponent as a marker).
    void EnemySystem::update(mario::engine::EntityManagerFacade& registry, const TileMap& map, float dt) const {
        static thread_local std::vector<EntityID> entities;
        // Query entities that have EnemyComponent (all required components should exist for valid enemies)
        registry.get_entities_with<EnemyComponent>(entities);

        for (auto entity : entities) {
            auto enemy_opt = registry.get_component<EnemyComponent>(entity);
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            auto coll_opt = registry.get_component<CollisionInfoComponent>(entity);
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);

            if (enemy_opt && vel_opt && coll_opt && pos_opt && size_opt) {
                auto& enemy = enemy_opt->get();
                auto& vel = vel_opt->get();
                auto& coll = coll_opt->get();
                auto& pos = pos_opt->get();
                auto& size = size_opt->get();
                // Only reverse direction on collision and velocity is significant
                constexpr float MIN_SPEED_THRES = 0.1f;
                float speed = std::abs(vel.vx);

                if (coll.collided && speed >= MIN_SPEED_THRES) {
                    vel.vx = -vel.vx;
                }

                // Constrain movement to the contiguous solid platform beneath the enemy
                const int tile_size = map.tile_size();
                if (tile_size <= 0) continue;

                // Calculate tile coordinates for the tile directly below the enemy's feet
                const float feet_x = pos.x + size.width * 0.5f; // center x
                const float feet_y = pos.y + size.height; // bottom y

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

                // Convert tile bounds to world coordinates
                const auto platform_left = static_cast<float>(left_tx * tile_size);
                const auto platform_right = static_cast<float>((right_tx + 1) * tile_size);

                // Check if enemy is approaching platform edge and reverse direction if needed
                constexpr float eps = 0.001f;
                const float next_x = pos.x + vel.vx * dt;

                if (vel.vx > 0.0f && (next_x + size.width) > (platform_right - eps)) {
                    if (std::abs(vel.vx) >= MIN_SPEED_THRES) vel.vx = -vel.vx;
                } else if (vel.vx < 0.0f && next_x < (platform_left + eps)) {
                    if (std::abs(vel.vx) >= MIN_SPEED_THRES) vel.vx = -vel.vx;
                }
            }
        }
    }
} // namespace mario
