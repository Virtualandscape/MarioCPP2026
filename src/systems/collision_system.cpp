#include <algorithm>
#include <cmath>
#include "mario/systems/CollisionSystem.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/CollisionInfoComponent.hpp"
#include "mario/ecs/components/TypeComponent.hpp"
#include "mario/ecs/components/PlayerControllerComponent.hpp"
#include "mario/ecs/EntityTypeComponent.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/helpers/quadtree.h"
#include "mario/helpers/tileSweep.hpp"

// collision_system.cpp
//
// This file implements the CollisionSystem for the Mario game, handling both tile and entity collisions.
// It uses a two-phase approach: first resolving collisions with the tile map (static world), then resolving collisions between entities (dynamic objects).
//
// Key concepts:
// - Broadphase: Uses a quadtree to efficiently find potential entity collision pairs.
// - Narrowphase: Performs precise AABB (axis-aligned bounding box) collision checks and applies gameplay-specific responses.
// - Tile collision: Uses swept AABB to resolve movement against the tile map.
//
// Main functions:
// - rects_intersect: Checks if two rectangles overlap.
// - resolve_player_collision: Adjusts player position and velocity to resolve overlap with another entity.
// - handle_entity_collision: Handles collision response between two entities, including player-specific logic.
// - CollisionSystem::update: Main update loop, processes all collisions for the current frame.

namespace mario {
    namespace {
        // Rectangle helpers: work directly with x,y,width,height instead of AABB struct
        // Returns true if two rectangles overlap.
        inline bool rects_intersect(float ax, float ay, float aw, float ah,
                                    float bx, float by, float bw, float bh) {
            return ax < (bx + bw) && (ax + aw) > bx && ay < (by + bh) && (ay + ah) > by;
        }

        // View struct to group all components needed for collision.
        struct CollidableView {
            std::reference_wrapper<PositionComponent> pos;
            std::reference_wrapper<SizeComponent> size;
            std::reference_wrapper<CollisionInfoComponent> coll;
            std::reference_wrapper<TypeComponent> type;
            std::optional<std::reference_wrapper<VelocityComponent>> vel;
        };

        // Build an SFML rect for broadphase queries.
        inline sf::FloatRect to_rect(const CollidableView& c) {
            return sf::FloatRect({c.pos.get().x, c.pos.get().y}, {c.size.get().width, c.size.get().height});
        }

        // Resolves overlap between the player and another entity, adjusting position and velocity.
        void resolve_player_collision(PositionComponent& pos_player, VelocityComponent& vel_player, const SizeComponent& size_player, const PositionComponent& pos_other, const SizeComponent& size_other) {
            // Compute the sides of both rectangles
            const float right_p = pos_player.x + size_player.width;
            const float bottom_p = pos_player.y + size_player.height;
            const float right_o = pos_other.x + size_other.width;
            const float bottom_o = pos_other.y + size_other.height;

            // Calculate overlap distances on each side
            const float overlapLeft = right_o - pos_player.x;
            const float overlapRight = right_p - pos_other.x;
            const float overlapTop = bottom_o - pos_player.y;
            const float overlapBottom = bottom_p - pos_other.y;

            // Find the minimum translation needed to resolve the collision
            const float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
            const float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

            // Move the player out of collision along the axis of least penetration
            if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                pos_player.x += minOverlapX;
                // Zero velocity if moving into the other entity
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

        // Performs narrowphase collision test and response between two entities.
        // Sets collision flags and applies player-specific resolution if needed.
        inline void handle_entity_collision(CollidableView& a, CollidableView& b) {
            // Early-out if AABBs do not overlap
            if (!rects_intersect(a.pos.get().x, a.pos.get().y, a.size.get().width, a.size.get().height,
                                 b.pos.get().x, b.pos.get().y, b.size.get().width, b.size.get().height)) {
                return;
            }

            // Mark both entities as collided and store the type of the other entity
            a.coll.get().collided = true;
            a.coll.get().other_type = b.type.get().type;
            b.coll.get().collided = true;
            b.coll.get().other_type = a.type.get().type;

            // If either entity is a player, resolve overlap with velocity response
            if (a.type.get().type == EntityTypeComponent::Player) {
                if (a.vel.has_value()) {
                    resolve_player_collision(a.pos.get(), a.vel->get(), a.size.get(), b.pos.get(), b.size.get());
                }
            } else if (b.type.get().type == EntityTypeComponent::Player) {
                if (b.vel.has_value()) {
                    resolve_player_collision(b.pos.get(), b.vel->get(), b.size.get(), a.pos.get(), a.size.get());
                }
            }
        }
     }

     // Main update function for the collision system.
    // 1. Resolves tile collisions for all entities with position, velocity, and size.
    // 2. Builds a list of collidable entities and inserts them into a quadtree for broadphase queries.
    // 3. For each entity, retrieves nearby candidates and performs narrowphase collision checks and responses.
    void CollisionSystem::update(EntityManager& registry, const TileMap& map, float dt) {
        // First, handle tile collisions for entities with Position, Velocity, Size
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PositionComponent>(entities);
        for (auto entity : entities) {
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);
            if (pos_opt && vel_opt && size_opt) {
                auto& pos = pos_opt->get();
                auto& vel = vel_opt->get();
                auto& size = size_opt->get();
                // Use swept AABB to resolve movement against the tile map, using helpers/TileSweep
                const float old_x = pos.x;
                const float old_y = pos.y;
                const float next_x = old_x + vel.vx * dt;
                const float next_y = old_y + vel.vy * dt;

                const auto result = resolve_tile_collision(next_x, next_y, vel.vx, vel.vy, size.width, size.height, map, dt);
                pos.x = result.x;
                pos.y = result.y;
                vel.vx = result.vx;
                vel.vy = result.vy;

                // Ground check for controller: reset jump_count when grounded and set on_ground flag
                if (auto ctrl_opt = registry.get_component<PlayerControllerComponent>(entity)) {
                    constexpr float epsilon = 0.1f;
                    const float bottom = pos.y + size.height;
                    const int ty = static_cast<int>(std::floor((bottom + epsilon) / static_cast<float>(map.tile_size())));
                    const int start_tx = static_cast<int>(std::floor(pos.x / static_cast<float>(map.tile_size())));
                    int end_tx = static_cast<int>(std::floor((pos.x + size.width - epsilon) / static_cast<float>(map.tile_size())));
                    if (end_tx < start_tx) end_tx = start_tx;

                    bool on_ground = false;
                    for (int tx = start_tx; tx <= end_tx; ++tx) {
                        if (map.is_solid(tx, ty)) {
                            on_ground = true;
                            break;
                        }
                    }
                    if (on_ground) {
                        auto &ctrl = ctrl_opt->get();
                        ctrl.jump_count = 0;
                        ctrl.on_ground = true;
                    } else {
                        ctrl_opt->get().on_ground = false;
                    }
                }
            }
        }

        // Then, handle entity vs entity collisions (broadphase via quadtree)
        static thread_local std::vector<EntityID> collidable_entities;
        registry.get_entities_with<PositionComponent>(collidable_entities);
        std::vector<CollidableView> collidables;
        collidables.reserve(collidable_entities.size());
        for (auto entity : collidable_entities) {
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);
            auto coll_opt = registry.get_component<CollisionInfoComponent>(entity);
            auto type_opt = registry.get_component<TypeComponent>(entity);
            if (!pos_opt || !size_opt || !coll_opt || !type_opt) continue;
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            CollidableView view{std::ref(pos_opt->get()), std::ref(size_opt->get()), std::ref(coll_opt->get()), std::ref(type_opt->get()), std::nullopt};
            if (vel_opt) view.vel = std::optional<std::reference_wrapper<VelocityComponent>>(std::ref(vel_opt->get()));
            collidables.push_back(std::move(view));
         }

         if (map.tile_size() <= 0 || collidables.empty()) {
             return;
         }

         // Build a quadtree covering the world for broadphase collision queries
         const auto world_w = static_cast<float>(map.width() * map.tile_size());
         const auto world_h = static_cast<float>(map.height() * map.tile_size());
         quadtree quadtree(0, sf::FloatRect({0.0f, 0.0f}, {world_w, world_h}));

         // Insert AABBs with their collidable index as payload.
         for (std::size_t i = 0; i < collidables.size(); ++i) {
             quadtree.insert(QuadTile(to_rect(collidables[i]), static_cast<std::uint32_t>(i)));
         }

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
