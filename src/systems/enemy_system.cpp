#include <cmath>
#include "mario/systems/EnemySystem.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/Velocity.hpp"
#include "mario/ecs/components/Position.hpp"
#include "mario/ecs/components/CollisionInfo.hpp"

namespace mario {
    void EnemySystem::update(Registry& registry, float dt) const {
        auto entities = registry.get_entities_with<EnemyComponent>();
        for (auto entity : entities) {
            auto* vel = registry.get_component<Velocity>(entity);
            auto* coll = registry.get_component<CollisionInfo>(entity);

            if (vel && coll) {
                // If hit something horizontally, reverse direction
                if (coll->collided) {
                    // Check if the collision was horizontal (crude check but better than before)
                    // If we collided and our velocity is 0 or reversed, it means we hit a wall/entity
                    if (std::abs(vel->vx) < 0.1f) {
                         // We don't know which way we were going if vx is 0,
                         // but usually they start moving left.
                         // For now, let's just make sure they move.
                         vel->vx = -50.0f;
                    }
                }

                // Ensure they are always moving
                if (vel->vx == 0.0f) {
                     vel->vx = -50.0f;
                }
            }
        }
    }
} // namespace mario
