#include <algorithm>
#include <cmath>
#include "mario/systems/CollisionSystem.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/CollisionInfoComponent.hpp"
#include "mario/ecs/components/TypeComponent.hpp"
#include "mario/ecs/EntityTypeComponent.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    namespace {
        // Small epsilon to separate resolved positions to avoid immediate re-collision
        constexpr float kCollisionEpsilon = 0.01f;

        // Rectangle helpers: work directly with x,y,width,height instead of AABB struct
        inline float rect_right(float x, float w) { return x + w; }
        inline float rect_bottom(float y, float h) { return y + h; }

        inline bool rects_intersect(float ax, float ay, float aw, float ah,
                                    float bx, float by, float bw, float bh) {
            return ax < (bx + bw) && (ax + aw) > bx && ay < (by + bh) && (ay + ah) > by;
        }

        struct CollisionResult {
            float x, y;
            float vx, vy;
        };

        CollisionResult resolve_tile_collision(float x, float y, float vx, float vy, float w, float h, const TileMap& map, float dt) {
            const float prev_x = x - vx * dt;
            const float prev_y = y - vy * dt;
            const int tile_size = map.tile_size();
            const auto tile_size_f = static_cast<float>(tile_size);

            if (tile_size <= 0) return {x, y, vx, vy};

            float new_x = x;
            float new_y = y;
            float new_vx = vx;
            float new_vy = vy;

            // X-axis
            if (new_vx != 0.0f) {
                const float min_x = std::min(prev_x, x);
                const float max_x = std::max(prev_x, x);
                const int start_x = static_cast<int>(std::floor(min_x / tile_size_f));
                const int end_x = static_cast<int>(std::floor((max_x + w) / tile_size_f));
                const int start_y = static_cast<int>(std::floor(prev_y / tile_size_f));
                const int end_y = static_cast<int>(std::floor((prev_y + h) / tile_size_f));

                for (int ty = start_y; ty <= end_y; ++ty) {
                    for (int tx = start_x; tx <= end_x; ++tx) {
                        if (!map.is_solid(tx, ty)) continue;
                        const float tile_left = static_cast<float>(tx) * tile_size_f;
                        const float tile_top = static_cast<float>(ty) * tile_size_f;
                        if (!rects_intersect(new_x, prev_y, w, h, tile_left, tile_top, tile_size_f, tile_size_f)) continue;

                        new_x = (new_vx > 0.0f) ? tile_left - w : tile_left + tile_size_f;
                        new_vx = 0.0f;
                    }
                }
            }

            // Y-axis
            if (new_vy != 0.0f) {
                const float min_y = std::min(prev_y, y);
                const float max_y = std::max(prev_y, y);
                const int start_x = static_cast<int>(std::floor(new_x / tile_size_f));
                const int end_x = static_cast<int>(std::floor((new_x + w) / tile_size_f));
                const int start_y = static_cast<int>(std::floor(min_y / tile_size_f));
                const int end_y = static_cast<int>(std::floor((max_y + h) / tile_size_f));

                for (int ty = start_y; ty <= end_y; ++ty) {
                    for (int tx = start_x; tx <= end_x; ++tx) {
                        if (!map.is_solid(tx, ty)) continue;
                        const float tile_left = static_cast<float>(tx) * tile_size_f;
                        const float tile_top = static_cast<float>(ty) * tile_size_f;
                        if (!rects_intersect(new_x, new_y, w, h, tile_left, tile_top, tile_size_f, tile_size_f)) continue;

                        new_y = (new_vy > 0.0f) ? tile_top - h : tile_top + tile_size_f;
                        new_vy = 0.0f;
                    }
                }
            }
            return {new_x, new_y, new_vx, new_vy};
        }
        void resolve_player_collision(PositionComponent& pos_player, VelocityComponent& vel_player, const SizeComponent& size_player, const PositionComponent& pos_other, const SizeComponent& size_other) {
            float left_p = pos_player.x;
            float top_p = pos_player.y;
            float right_p = rect_right(pos_player.x, size_player.width);
            float bottom_p = rect_bottom(pos_player.y, size_player.height);

            float left_o = pos_other.x;
            float top_o = pos_other.y;
            float right_o = rect_right(pos_other.x, size_other.width);
            float bottom_o = rect_bottom(pos_other.y, size_other.height);

            float overlapLeft = right_o - left_p;
            float overlapRight = right_p - left_o;
            float overlapTop = bottom_o - top_p;
            float overlapBottom = bottom_p - top_o;

            float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
            float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

            if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                pos_player.x += minOverlapX;
                // Only zero out velocity if we actually hit a vertical wall (not just brushing past an entity)
                // And only if we are moving TOWARDS the wall
                if ((minOverlapX < 0 && vel_player.vx > 0) || (minOverlapX > 0 && vel_player.vx < 0)) {
                    vel_player.vx = 0.0f;
                }
            } else {
                pos_player.y += minOverlapY;
                // If we hit from above (minOverlapY is negative, player is above other), it's a stomp!
                // Or if we hit from below (minOverlapY is positive), we stop upward velocity.
                if (minOverlapY < 0 && vel_player.vy > 0) {
                    vel_player.vy = 0.0f;
                    // Potential stomp logic could go here or in a separate system
                } else if (minOverlapY > 0 && vel_player.vy < 0) {
                    vel_player.vy = 0.0f;
                }
            }
        }
    }

    // Check if the entity collides with any tiles in the tile map
    void check_entity_tile_collision(PositionComponent& pos, VelocityComponent& vel, const SizeComponent& size, const TileMap& map, float dt) {
        auto result = resolve_tile_collision(pos.x, pos.y, vel.vx, vel.vy, size.width, size.height, map, dt);
        pos.x = result.x;
        pos.y = result.y;
        vel.vx = result.vx;
        vel.vy = result.vy;
    }

    void CollisionSystem::update(Registry& registry, const TileMap& map, float dt) {
        // First, handle tile collisions for entities with Position, Velocity, Size
        auto entities = registry.get_entities_with<PositionComponent>();
        for (auto entity : entities) {
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* vel = registry.get_component<VelocityComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);
            if (pos && vel && size) {
                check_entity_tile_collision(*pos, *vel, *size, map, dt);
            }
        }

        // Then, handle entity vs entity collisions
        // Get all entities with Position, Size, CollisionInfo, Type
        auto collidable_entities = registry.get_entities_with<PositionComponent>();

        for (size_t i = 0; i < collidable_entities.size(); ++i) {
            auto entity_a = collidable_entities[i];
            auto* pos_a = registry.get_component<PositionComponent>(entity_a);
            auto* size_a = registry.get_component<SizeComponent>(entity_a);
            auto* coll_a = registry.get_component<CollisionInfoComponent>(entity_a);
            auto* type_a = registry.get_component<TypeComponent>(entity_a);
            if (!pos_a || !size_a || !coll_a || !type_a) continue;

            for (size_t j = i + 1; j < collidable_entities.size(); ++j) {
                auto entity_b = collidable_entities[j];
                auto* pos_b = registry.get_component<PositionComponent>(entity_b);
                auto* size_b = registry.get_component<SizeComponent>(entity_b);
                auto* coll_b = registry.get_component<CollisionInfoComponent>(entity_b);
                auto* type_b = registry.get_component<TypeComponent>(entity_b);
                if (!pos_b || !size_b || !coll_b || !type_b) continue;

                const float left_a = pos_a->x;
                const float top_a = pos_a->y;
                const float right_a = rect_right(pos_a->x, size_a->width);
                const float bottom_a = rect_bottom(pos_a->y, size_a->height);

                const float left_b = pos_b->x;
                const float top_b = pos_b->y;
                const float right_b = rect_right(pos_b->x, size_b->width);
                const float bottom_b = rect_bottom(pos_b->y, size_b->height);

                if (left_a < right_b && right_a > left_b && top_a < bottom_b && bottom_a > top_b) {
                    coll_a->collided = true;
                    coll_a->other_type = type_b->type;
                    coll_b->collided = true;
                    coll_b->other_type = type_a->type;

                    // Special resolution for player
                    if (type_a->type == EntityTypeComponent::Player) {
                        auto* vel_a = registry.get_component<VelocityComponent>(entity_a);
                        if (vel_a) resolve_player_collision(*pos_a, *vel_a, *size_a, *pos_b, *size_b);
                    } else if (type_b->type == EntityTypeComponent::Player) {
                        auto* vel_b = registry.get_component<VelocityComponent>(entity_b);
                        if (vel_b) resolve_player_collision(*pos_b, *vel_b, *size_b, *pos_a, *size_a);
                    }
                }
            }
        }
    }
}

