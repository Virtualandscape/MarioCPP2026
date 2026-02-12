#include "mario/systems/PhysicsSystems.hpp"
#include "mario/ecs/Components.hpp"
#include "mario/world/World.hpp"
#include "mario/helpers/tileSweep.hpp"
#include <vector>
#include <cmath>

namespace mario::systems::physics {

    void update(EntityManager& registry, float dt, float gravity) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PositionComponent, VelocityComponent>(entities);
        for (auto entity : entities) {
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            if (pos_opt && vel_opt) {
                auto& vel = vel_opt->get();
                // Apply gravity acceleration to vertical velocity
                vel.vy += gravity * dt;
                // Note: Position integration is handled by the CollisionSystem after resolving tile collisions
            }
        }
    }

    namespace {
        // Returns true if two rectangles overlap.
        inline bool rects_intersect(float ax, float ay, float aw, float ah,
                                    float bx, float by, float bw, float bh) {
            return ax < (bx + bw) && (ax + aw) > bx && ay < (by + bh) && (ay + ah) > by;
        }


        void resolve_player_collision(PositionComponent& pos_player, VelocityComponent& vel_player, const SizeComponent& size_player, const PositionComponent& pos_other, const SizeComponent& size_other) {
            const float right_p = pos_player.x + size_player.width;
            const float bottom_p = pos_player.y + size_player.height;
            const float right_o = pos_other.x + size_other.width;
            const float bottom_o = pos_other.y + size_other.height;

            const float overlapLeft = right_o - pos_player.x;
            const float overlapRight = right_p - pos_other.x;
            const float overlapTop = bottom_o - pos_player.y;
            const float overlapBottom = bottom_p - pos_other.y;

            const float minOverlapX = (std::abs(overlapLeft) < std::abs(overlapRight)) ? overlapLeft : -overlapRight;
            const float minOverlapY = (std::abs(overlapTop) < std::abs(overlapBottom)) ? overlapTop : -overlapBottom;

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
    }

    void resolve_collisions(EntityManager& registry, const TileMap& map, float dt) {
        // ... Logic integrated from collision_system.cpp ...
        // (Simplified for brevity in the actual response, but keeping the implementation logic)

        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with_all<PositionComponent, SizeComponent, VelocityComponent>(entities);

        // Phase 1: Tile Collision (static world)
        for (auto entity : entities) {
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);
            auto vel_opt = registry.get_component<VelocityComponent>(entity);

            if (pos_opt && size_opt && vel_opt) {
                auto& pos = pos_opt->get();
                auto& size = size_opt->get();
                auto& vel = vel_opt->get();

                // Call helper to resolve against tiles
                auto result = resolve_tile_collision(pos.x, pos.y, vel.vx, vel.vy, size.width, size.height, map, dt);

                pos.x = result.x;
                pos.y = result.y;
                vel.vx = result.vx;
                vel.vy = result.vy;
            }
        }

        // Phase 2: Entity Collision (dynamic)
        static thread_local std::vector<EntityID> collidable_ids;
        registry.get_entities_with_all<PositionComponent, SizeComponent, CollisionInfoComponent, TypeComponent>(collidable_ids);

        // Use Quadtree for broadphase if many entities, or simple O(N^2) for fewer.
        for (size_t i = 0; i < collidable_ids.size(); ++i) {
            for (size_t j = i + 1; j < collidable_ids.size(); ++j) {
                EntityID idA = collidable_ids[i];
                EntityID idB = collidable_ids[j];

                auto posA = registry.get_component<PositionComponent>(idA);
                auto sizeA = registry.get_component<SizeComponent>(idA);
                auto typeA = registry.get_component<TypeComponent>(idA);
                auto collA = registry.get_component<CollisionInfoComponent>(idA);

                auto posB = registry.get_component<PositionComponent>(idB);
                auto sizeB = registry.get_component<SizeComponent>(idB);
                auto typeB = registry.get_component<TypeComponent>(idB);
                auto collB = registry.get_component<CollisionInfoComponent>(idB);

                if (posA && sizeA && posB && sizeB && typeA && typeB && collA && collB) {
                    if (rects_intersect(posA->get().x, posA->get().y, sizeA->get().width, sizeA->get().height,
                                        posB->get().x, posB->get().y, sizeB->get().width, sizeB->get().height)) {

                        collA->get().collided = true;
                        collA->get().other_type = typeB->get().type;
                        collB->get().collided = true;
                        collB->get().other_type = typeA->get().type;

                        // Example: Player vs Enemy resolution
                        if (typeA->get().type == EntityType::Player) {
                            auto velA = registry.get_component<VelocityComponent>(idA);
                            if (velA) resolve_player_collision(posA->get(), velA->get(), sizeA->get(), posB->get(), sizeB->get());
                        } else if (typeB->get().type == EntityType::Player) {
                            auto velB = registry.get_component<VelocityComponent>(idB);
                            if (velB) resolve_player_collision(posB->get(), velB->get(), sizeB->get(), posA->get(), sizeA->get());
                        }
                    }
                }
            }
        }
    }
}

