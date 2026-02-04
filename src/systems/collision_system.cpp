#include <algorithm>
#include <cmath>
#include <unordered_set>
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

        struct CollidableView {
            EntityID id;
            PositionComponent* pos;
            SizeComponent* size;
            CollisionInfoComponent* coll;
            TypeComponent* type;
            VelocityComponent* vel;
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

    void CollisionSystem::update(EntityManager& registry, const TileMap& map, float dt) {
        // First, handle tile collisions for entities with Position, Velocity, Size
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PositionComponent>(entities);
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
        static thread_local std::vector<EntityID> collidable_entities;
        registry.get_entities_with<PositionComponent>(collidable_entities);
        std::vector<CollidableView> collidables;
        collidables.reserve(collidable_entities.size());
        for (auto entity : collidable_entities) {
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);
            auto* coll = registry.get_component<CollisionInfoComponent>(entity);
            auto* type = registry.get_component<TypeComponent>(entity);
            if (!pos || !size || !coll || !type) continue;
            collidables.push_back({entity, pos, size, coll, type, registry.get_component<VelocityComponent>(entity)});
        }

        const float cell_size = static_cast<float>(map.tile_size());
        const int cols = std::max(1, map.width());
        const int rows = std::max(1, map.height());
        const int bucket_count = cols * rows;

        static thread_local std::vector<std::vector<std::size_t>> buckets;
        if (static_cast<int>(buckets.size()) != bucket_count) {
            buckets.assign(static_cast<std::size_t>(bucket_count), {});
        } else {
            for (auto& bucket : buckets) {
                bucket.clear();
            }
        }

        if (cell_size > 0.0f) {
            for (std::size_t i = 0; i < collidables.size(); ++i) {
                const auto& c = collidables[i];
                const float left = c.pos->x;
                const float top = c.pos->y;
                const float right = rect_right(c.pos->x, c.size->width);
                const float bottom = rect_bottom(c.pos->y, c.size->height);

                int min_cx = static_cast<int>(std::floor(left / cell_size));
                int max_cx = static_cast<int>(std::floor((right - 0.001f) / cell_size));
                int min_cy = static_cast<int>(std::floor(top / cell_size));
                int max_cy = static_cast<int>(std::floor((bottom - 0.001f) / cell_size));

                min_cx = std::clamp(min_cx, 0, cols - 1);
                max_cx = std::clamp(max_cx, 0, cols - 1);
                min_cy = std::clamp(min_cy, 0, rows - 1);
                max_cy = std::clamp(max_cy, 0, rows - 1);

                for (int cy = min_cy; cy <= max_cy; ++cy) {
                    for (int cx = min_cx; cx <= max_cx; ++cx) {
                        buckets[static_cast<std::size_t>(cy * cols + cx)].push_back(i);
                    }
                }
            }
        }

        std::unordered_set<std::uint64_t> seen;
        seen.reserve(collidables.size() * 4u + 8u);

        for (const auto& bucket : buckets) {
            const std::size_t count = bucket.size();
            for (std::size_t bi = 0; bi + 1 < count; ++bi) {
                for (std::size_t bj = bi + 1; bj < count; ++bj) {
                    const std::size_t ia = bucket[bi];
                    const std::size_t ib = bucket[bj];
                    const std::size_t min_i = std::min(ia, ib);
                    const std::size_t max_i = std::max(ia, ib);
                    const std::uint64_t key = (static_cast<std::uint64_t>(min_i) << 32u) | static_cast<std::uint64_t>(max_i);
                    if (!seen.insert(key).second) continue;

                    auto& a = collidables[min_i];
                    auto& b = collidables[max_i];

                    const float left_a = a.pos->x;
                    const float top_a = a.pos->y;
                    const float right_a = rect_right(a.pos->x, a.size->width);
                    const float bottom_a = rect_bottom(a.pos->y, a.size->height);

                    const float left_b = b.pos->x;
                    const float top_b = b.pos->y;
                    const float right_b = rect_right(b.pos->x, b.size->width);
                    const float bottom_b = rect_bottom(b.pos->y, b.size->height);

                    if (left_a < right_b && right_a > left_b && top_a < bottom_b && bottom_a > top_b) {
                        a.coll->collided = true;
                        a.coll->other_type = b.type->type;
                        b.coll->collided = true;
                        b.coll->other_type = a.type->type;

                        if (a.type->type == EntityTypeComponent::Player) {
                            if (a.vel) resolve_player_collision(*a.pos, *a.vel, *a.size, *b.pos, *b.size);
                        } else if (b.type->type == EntityTypeComponent::Player) {
                            if (b.vel) resolve_player_collision(*b.pos, *b.vel, *b.size, *a.pos, *a.size);
                        }
                    }
                }
            }
        }
    }
}

