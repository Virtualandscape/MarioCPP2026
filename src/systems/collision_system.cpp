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
#include "mario/util/Quadtree.h"

namespace mario {
    namespace {
        // Rectangle helpers: work directly with x,y,width,height instead of AABB struct
        inline bool rects_intersect(float ax, float ay, float aw, float ah,
                                    float bx, float by, float bw, float bh) {
            return ax < (bx + bw) && (ax + aw) > bx && ay < (by + bh) && (ay + ah) > by;
        }

        struct CollisionResult {
            float x, y;
            float vx, vy;
        };

        struct CollidableView {
            PositionComponent* pos;
            SizeComponent* size;
            CollisionInfoComponent* coll;
            TypeComponent* type;
            VelocityComponent* vel;
        };

        // Build an SFML rect for broadphase queries.
        inline sf::FloatRect to_rect(const CollidableView& c) {
            return sf::FloatRect({c.pos->x, c.pos->y}, {c.size->width, c.size->height});
        }

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

            // Resolve one axis at a time using swept AABB against solid tiles.
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

                // Scan the tile range intersected by the swept bounds.
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

                        // Snap to tile edge and zero velocity along this axis.
                        if (horizontal) {
                            new_pos = (new_vel > 0.0f) ? tile_left - size_axis : tile_left + tile_size_f;
                        } else {
                            new_pos = (new_vel > 0.0f) ? tile_top - size_axis : tile_top + tile_size_f;
                        }
                        new_vel = 0.0f;
                    }
                }
            };

            resolve_axis(new_x, new_vx, prev_x, prev_y, w, h, true);
            resolve_axis(new_y, new_vy, prev_y, new_x, h, w, false);

            return {new_x, new_y, new_vx, new_vy};
        }
        // Resolve player overlap against another entity (with velocity response).
        void resolve_player_collision(PositionComponent& pos_player, VelocityComponent& vel_player, const SizeComponent& size_player, const PositionComponent& pos_other, const SizeComponent& size_other) {
            const float right_p = pos_player.x + size_player.width;
            const float bottom_p = pos_player.y + size_player.height;
            const float right_o = pos_other.x + size_other.width;
            const float bottom_o = pos_other.y + size_other.height;

            const float overlapLeft = right_o - pos_player.x;
            const float overlapRight = right_p - pos_other.x;
            const float overlapTop = bottom_o - pos_player.y;
            const float overlapBottom = bottom_p - pos_other.y;

            const float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
            const float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

            if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                pos_player.x += minOverlapX;
                if ((minOverlapX < 0 && vel_player.vx > 0) || (minOverlapX > 0 && vel_player.vx < 0)) {
                    vel_player.vx = 0.0f;
                }
            } else {
                pos_player.y += minOverlapY;
                if ((minOverlapY < 0 && vel_player.vy > 0) || (minOverlapY > 0 && vel_player.vy < 0)) {
                    vel_player.vy = 0.0f;
                }
            }
        }

        // Narrowphase: AABB test + collision flags + optional player resolution.
        inline void handle_entity_collision(CollidableView& a, CollidableView& b) {
            if (!rects_intersect(a.pos->x, a.pos->y, a.size->width, a.size->height,
                                 b.pos->x, b.pos->y, b.size->width, b.size->height)) {
                return;
            }

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

    //
    void CollisionSystem::update(EntityManager& registry, const TileMap& map, float dt) {
        // First, handle tile collisions for entities with Position, Velocity, Size
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PositionComponent>(entities);
        for (auto entity : entities) {
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* vel = registry.get_component<VelocityComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);
            if (pos && vel && size) {
                auto result = resolve_tile_collision(pos->x, pos->y, vel->vx, vel->vy, size->width, size->height, map, dt);
                pos->x = result.x;
                pos->y = result.y;
                vel->vx = result.vx;
                vel->vy = result.vy;
            }
        }

        // Then, handle entity vs entity collisions (broadphase via quadtree)
        static thread_local std::vector<EntityID> collidable_entities;
        registry.get_entities_with<PositionComponent>(collidable_entities);
        std::vector<CollidableView> collidables;
        collidables.reserve(collidable_entities.size());
        // Loop over collidable entities and build collidable views.
        for (auto entity : collidable_entities) {
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);
            auto* coll = registry.get_component<CollisionInfoComponent>(entity);
            auto* type = registry.get_component<TypeComponent>(entity);
            if (!pos || !size || !coll || !type) continue;
            collidables.push_back({pos, size, coll, type, registry.get_component<VelocityComponent>(entity)});
        }

        if (map.tile_size() <= 0 || collidables.empty()) {
            return;
        }

        const auto world_w = static_cast<float>(map.width() * map.tile_size());
        const auto world_h = static_cast<float>(map.height() * map.tile_size());
        Quadtree quadtree(0, sf::FloatRect({0.0f, 0.0f}, {world_w, world_h}));

        // Insert AABBs with their collidable index as payload.
        for (std::size_t i = 0; i < collidables.size(); ++i) {
            quadtree.insert(QuadTile(to_rect(collidables[i]), static_cast<std::uint32_t>(i)));
        }
        // Iterate over collidables and query nearby tiles to test.
        static thread_local std::vector<QuadTile> candidates;
        for (std::size_t i = 0; i < collidables.size(); ++i) {
            const auto& c = collidables[i];
            candidates.clear();
            // Broadphase query: return only nearby candidates.
            quadtree.retrieve(candidates, to_rect(c));

            for (const auto& tile : candidates) {
                const auto other_index = static_cast<std::size_t>(tile.id);
                if (other_index <= i || other_index >= collidables.size()) continue;
                // Narrowphase: AABB + gameplay-specific response.
                handle_entity_collision(collidables[i], collidables[other_index]);
            }
        }
    }
}
