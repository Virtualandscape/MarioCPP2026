#include <cmath>
#include "mario/systems/EnemySystem.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/CollisionInfoComponent.hpp"

namespace mario {
    void EnemySystem::update(Registry& registry, float dt) const {
        auto entities = registry.get_entities_with<EnemyComponent>();
        for (auto entity : entities) {
            auto* enemy = registry.get_component<EnemyComponent>(entity);
            auto* vel = registry.get_component<VelocityComponent>(entity);
            auto* coll = registry.get_component<CollisionInfoComponent>(entity);

            if (enemy && vel && coll) {
                // If collided, reverse horizontal direction while keeping magnitude = move_speed
                if (coll->collided) {
                    if (std::abs(vel->vx) < 0.1f) {
                        // Unknown direction -> default to left
                        vel->vx = -enemy->move_speed;
                    } else {
                        // Reverse direction but keep consistent speed
                        vel->vx = (vel->vx > 0.0f) ? -enemy->move_speed : enemy->move_speed;
                    }
                }

                // Ensure they are always moving with the correct magnitude
                if (std::abs(vel->vx) < 0.1f) {
                    vel->vx = -enemy->move_speed; // default left
                } else {
                    // Normalize magnitude to enemy->move_speed
                    vel->vx = (vel->vx > 0.0f) ? enemy->move_speed : -enemy->move_speed;
                }
            }
        }
    }
} // namespace mario
