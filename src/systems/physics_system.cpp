// Implements the PhysicsSystem, which applies gravity and updates position and velocity for all entities with those components.
// Handles basic physics integration for movement.

#include "mario/systems/PhysicsSystem.hpp"

#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"

namespace mario {

void PhysicsSystem::update(EntityManager& registry, float dt) const
{
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<PositionComponent>(entities);
    for (auto entity : entities) {
        auto* pos = registry.get_component<PositionComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);
        if (vel) {
            vel->vy += _gravity * dt;
        }
    }
}

void PhysicsSystem::set_gravity(float g) { _gravity = g; }

} // namespace mario
