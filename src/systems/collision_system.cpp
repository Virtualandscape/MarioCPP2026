#include <algorithm>
#include <cmath>
#include "mario/systems/CollisionSystem.hpp"
#include "mario/entities/Entity.hpp"
#include "mario/ecs/components/Position.hpp"
#include "mario/ecs/components/Velocity.hpp"
#include "mario/ecs/components/Size.hpp"
#include "mario/ecs/components/CollisionInfo.hpp"
#include "mario/ecs/components/Type.hpp"
#include "mario/ecs/EntityType.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    namespace {
        // Small epsilon to separate resolved positions to avoid immediate re-collision
        constexpr float kCollisionEpsilon = 1.0f;

        // Rectangle helpers: work directly with x,y,width,height instead of AABB struct
        inline float rect_right(float x, float w) { return x + w; }
        inline float rect_bottom(float y, float h) { return y + h; }

        inline bool rects_intersect(float ax, float ay, float aw, float ah,
                                    float bx, float by, float bw, float bh) {
            return ax < (bx + bw) && (ax + aw) > bx && ay < (by + bh) && (ay + ah) > by;
        }
    }

    // Check if the entity collides with any tiles in the tile map
    void check_entity_tile_collision(Position& pos, Velocity& vel, const Size& size, const TileMap& map, float dt) {
        const float w = size.width;
        const float h = size.height;
        const float current_x = pos.x;
        const float current_y = pos.y;
        const float prev_x = current_x - vel.vx * dt;
        const float prev_y = current_y - vel.vy * dt;
        const int tile_size = map.tile_size();
        const auto tile_size_f = static_cast<float>(tile_size);

        if (tile_size <= 0) {
            return;
        }

        float new_x = current_x;
        float new_y = current_y;
        float vx = vel.vx;
        float vy = vel.vy;

        // On the x-axis
        if (vx != 0.0f) {
            const float min_x = std::min(prev_x, current_x);
            const float max_x = std::max(prev_x, current_x);
            const int start_x = static_cast<int>(std::floor(min_x / tile_size_f));
            const int end_x = static_cast<int>(std::floor((max_x + w) / tile_size_f));
            const int start_y = static_cast<int>(std::floor(prev_y / tile_size_f));
            const int end_y = static_cast<int>(std::floor((prev_y + h) / tile_size_f));

            // Iterate over tiles that the entity might collide with
            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) {
                        continue;
                    }
                    // Build collision detection for the entity with the tile
                    const float tile_left = static_cast<float>(tx) * tile_size_f;
                    const float tile_top = static_cast<float>(ty) * tile_size_f;
                    if (!rects_intersect(new_x, prev_y, w, h, tile_left, tile_top, tile_size_f, tile_size_f)) {
                        continue;
                    }

                    // If the entity is moving right, stop at the left edge of the tile
                    if (vx > 0.0f) {
                        new_x = tile_left - w;
                        // If the entity is moving left, stop at the right edge of the tile
                    } else {
                        new_x = tile_left + tile_size_f;
                    }
                    vx = 0.0f;
                }
            }
        }
        // On the y-axis
        if (vy != 0.0f) {
            const float min_y = std::min(prev_y, current_y);
            const float max_y = std::max(prev_y, current_y);
            const int start_x = static_cast<int>(std::floor(new_x / tile_size_f));
            const int end_x = static_cast<int>(std::floor((new_x + w) / tile_size_f));
            const int start_y = static_cast<int>(std::floor(min_y / tile_size_f));
            const int end_y = static_cast<int>(std::floor((max_y + h) / tile_size_f));

            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) {
                        continue;
                    }

                    const float tile_left = static_cast<float>(tx) * tile_size_f;
                    const float tile_top = static_cast<float>(ty) * tile_size_f;
                    if (!rects_intersect(new_x, new_y, w, h, tile_left, tile_top, tile_size_f, tile_size_f)) {
                        continue;
                    }

                    if (vy > 0.0f) {
                        new_y = tile_top - h;
                    } else {
                        new_y = tile_top + tile_size_f;
                    }
                    vy = 0.0f;
                }
            }
        }

        pos.x = new_x;
        pos.y = new_y;
        vel.vx = vx;
        vel.vy = vy;
    }

    void CollisionSystem::update(Registry& registry, const TileMap& map, float dt) {
        // First, handle tile collisions for entities with Position, Velocity, Size
        auto entities = registry.get_entities_with<Position>();
        for (auto entity : entities) {
            auto* pos = registry.get_component<Position>(entity);
            auto* vel = registry.get_component<Velocity>(entity);
            auto* size = registry.get_component<Size>(entity);
            if (pos && vel && size) {
                check_entity_tile_collision(*pos, *vel, *size, map, dt);
            }
        }

        // Then, handle entity vs entity collisions
        // Get all entities with Position, Size, CollisionInfo, Type
        auto collidable_entities = registry.get_entities_with<Position>();

        for (size_t i = 0; i < collidable_entities.size(); ++i) {
            auto entity_a = collidable_entities[i];
            auto* pos_a = registry.get_component<Position>(entity_a);
            auto* size_a = registry.get_component<Size>(entity_a);
            auto* coll_a = registry.get_component<CollisionInfo>(entity_a);
            auto* type_a = registry.get_component<Type>(entity_a);
            if (!pos_a || !size_a || !coll_a || !type_a) continue;

            for (size_t j = i + 1; j < collidable_entities.size(); ++j) {
                auto entity_b = collidable_entities[j];
                auto* pos_b = registry.get_component<Position>(entity_b);
                auto* size_b = registry.get_component<Size>(entity_b);
                auto* coll_b = registry.get_component<CollisionInfo>(entity_b);
                auto* type_b = registry.get_component<Type>(entity_b);
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
                    if (type_a->type == EntityType::Player) {
                        // Calculate intersection depths (signed)
                        float overlapLeft = right_b - left_a;
                        float overlapRight = right_a - left_b;
                        float overlapTop = bottom_b - top_a;
                        float overlapBottom = bottom_a - top_b;

                        // Find the smallest overlap on x and y axes with sign
                        float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
                        float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

                        // Resolve along the axis of least penetration
                        if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                            // Move player horizontally out of collision
                            pos_a->x += minOverlapX + ((minOverlapX > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon);
                            // Stop horizontal velocity
                            auto* vel_a = registry.get_component<Velocity>(entity_a);
                            if (vel_a) vel_a->vx = 0.0f;
                        } else {
                            // Move the player vertically out of collision
                            pos_a->y += minOverlapY + ((minOverlapY > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon);
                            // Stop vertical velocity
                            auto* vel_a = registry.get_component<Velocity>(entity_a);
                            if (vel_a) vel_a->vy = 0.0f;
                        }
                    } else if (type_b->type == EntityType::Player) {
                        // Similar for b as player
                        float overlapLeft = right_a - left_b;
                        float overlapRight = right_b - left_a;
                        float overlapTop = bottom_a - top_b;
                        float overlapBottom = bottom_b - top_a;

                        float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
                        float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

                        if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                            pos_b->x += minOverlapX + ((minOverlapX > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon);
                            auto* vel_b = registry.get_component<Velocity>(entity_b);
                            if (vel_b) vel_b->vx = 0.0f;
                        } else {
                            pos_b->y += minOverlapY + ((minOverlapY > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon);
                            auto* vel_b = registry.get_component<Velocity>(entity_b);
                            if (vel_b) vel_b->vy = 0.0f;
                        }
                    }
                }
            }
        }
    }


    void CollisionSystem::check_entity_collision(Entity &entity, const TileMap &map, float dt) {
        // Clear previous collision info
        entity.clear_collision_info();

        const float w = entity.width();
        const float h = entity.height();
        const float current_x = entity.x();
        const float current_y = entity.y();
        const float prev_x = current_x - entity.vx() * dt;
        const float prev_y = current_y - entity.vy() * dt;
        const int tile_size = map.tile_size();
        const auto tile_size_f = static_cast<float>(tile_size);

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
            const int start_x = static_cast<int>(std::floor(min_x / tile_size_f));
            const int end_x = static_cast<int>(std::floor((max_x + w) / tile_size_f));
            const int start_y = static_cast<int>(std::floor(prev_y / tile_size_f));
            const int end_y = static_cast<int>(std::floor((prev_y + h) / tile_size_f));

            // Iterate over tiles that the entity might collide with
            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) {
                        continue;
                    }
                    // Build collision detection for the entity with the tile
                    const float tile_left = static_cast<float>(tx) * tile_size_f;
                    const float tile_top = static_cast<float>(ty) * tile_size_f;
                    if (!rects_intersect(new_x, prev_y, w, h, tile_left, tile_top, tile_size_f, tile_size_f)) {
                        continue;
                    }

                    // If the entity is moving right, stop at the left edge of the tile
                    if (vx > 0.0f) {
                        new_x = tile_left - w;
                        // If the entity is moving left, stop at the right edge of the tile
                    } else {
                        new_x = tile_left + tile_size_f;
                    }
                    vx = 0.0f;
                }
            }
        }
        // On the y-axis
        if (vy != 0.0f) {
            const float min_y = std::min(prev_y, current_y);
            const float max_y = std::max(prev_y, current_y);
            const int start_x = static_cast<int>(std::floor(new_x / tile_size_f));
            const int end_x = static_cast<int>(std::floor((new_x + w) / tile_size_f));
            const int start_y = static_cast<int>(std::floor(min_y / tile_size_f));
            const int end_y = static_cast<int>(std::floor((max_y + h) / tile_size_f));

            for (int ty = start_y; ty <= end_y; ++ty) {
                for (int tx = start_x; tx <= end_x; ++tx) {
                    if (!map.is_solid(tx, ty)) {
                        continue;
                    }

                    const float tile_left = static_cast<float>(tx) * tile_size_f;
                    const float tile_top = static_cast<float>(ty) * tile_size_f;
                    if (!rects_intersect(new_x, new_y, w, h, tile_left, tile_top, tile_size_f, tile_size_f)) {
                        continue;
                    }

                    if (vy > 0.0f) {
                        new_y = tile_top - h;
                    } else {
                        new_y = tile_top + tile_size_f;
                    }
                    vy = 0.0f;
                }
            }
        }

        entity.set_position(new_x, new_y);
        entity.set_velocity(vx, vy);
    }

    void CollisionSystem::check_entity_vs_entity_collision(Entity &a, Entity &b, float dt) {
        const float left_a = a.x();
        const float top_a = a.y();
        const float right_a = rect_right(a.x(), a.width());
        const float bottom_a = rect_bottom(a.y(), a.height());

        const float left_b = b.x();
        const float top_b = b.y();
        const float right_b = rect_right(b.x(), b.width());
        const float bottom_b = rect_bottom(b.y(), b.height());

        bool collided = (left_a < right_b && right_a > left_b && top_a < bottom_b && bottom_a > top_b);

        // Broadphase to catch tunneling: build AABB that spans previous and current positions
        if (!collided && dt > 0.0f) {
            const float prev_left_a = a.x() - a.vx() * dt;
            const float prev_top_a = a.y() - a.vy() * dt;
            const float prev_right_a = prev_left_a + a.width();
            const float prev_bottom_a = prev_top_a + a.height();

            const float prev_left_b = b.x() - b.vx() * dt;
            const float prev_top_b = b.y() - b.vy() * dt;
            const float prev_right_b = prev_left_b + b.width();
            const float prev_bottom_b = prev_top_b + b.height();

            const float broad_left_a = std::min(prev_left_a, left_a);
            const float broad_top_a = std::min(prev_top_a, top_a);
            const float broad_right_a = std::max(prev_right_a, right_a);
            const float broad_bottom_a = std::max(prev_bottom_a, bottom_a);

            const float broad_left_b = std::min(prev_left_b, left_b);
            const float broad_top_b = std::min(prev_top_b, top_b);
            const float broad_right_b = std::max(prev_right_b, right_b);
            const float broad_bottom_b = std::max(prev_bottom_b, bottom_b);

            collided = (broad_left_a < broad_right_b && broad_right_a > broad_left_b && broad_top_a < broad_bottom_b && broad_bottom_a > broad_top_b);
        }

        if (collided) {
            a.set_collision_info({true, b.type()});
            b.set_collision_info({true, a.type()});

            // Resolve only when one of the entities is the Player (preserve existing behavior)
            if (a.type() == EntityType::Player) {
                // Calculate intersection depths (signed)
                float overlapLeft = right_b - left_a;
                float overlapRight = right_a - left_b;
                float overlapTop = bottom_b - top_a;
                float overlapBottom = bottom_a - top_b;

                // Find the smallest overlap on x and y axes with sign
                float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
                float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

                // Resolve along the axis of least penetration
                if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                    // Move player horizontally out of collision
                    a.set_position(a.x() + minOverlapX + ((minOverlapX > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon), a.y());
                    // Stop horizontal velocity
                    a.set_velocity(0.0f, a.vy());
                } else {
                    // Move the player vertically out of collision
                    a.set_position(a.x(), a.y() + minOverlapY + ((minOverlapY > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon));
                    // Stop vertical velocity
                    a.set_velocity(a.vx(), 0.0f);
                }

            } else if (b.type() == EntityType::Player) {
                // Similar for b as player
                float overlapLeft = right_a - left_b;
                float overlapRight = right_b - left_a;
                float overlapTop = bottom_a - top_b;
                float overlapBottom = bottom_b - top_a;

                float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
                float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;

                if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                    b.set_position(b.x() + minOverlapX + ((minOverlapX > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon), b.y());
                    // Stop horizontal velocity on the object
                    b.set_velocity(0.0f, b.vy());
                } else {
                    b.set_position(b.x(), b.y() + minOverlapY + ((minOverlapY > 0.0f) ? kCollisionEpsilon : -kCollisionEpsilon));
                    b.set_velocity(b.vx(), 0.0f);
                }
            }
        }
    }
}

