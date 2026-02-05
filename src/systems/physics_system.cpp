// Implements the PhysicsSystem, which applies gravity and integrates velocity into position for all entities.
// Handles basic physics integration for movement and gravitational acceleration.

#include "mario/systems/PhysicsSystem.hpp"

#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"

namespace mario {

void PhysicsSystem::update(EntityManager& registry, float dt) const
{
    // Query entities that have both Position and Velocity components
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<PositionComponent, VelocityComponent>(entities);
    for (auto entity : entities) {
        auto* pos = registry.get_component<PositionComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);
        if (pos && vel) {
            // Apply gravity acceleration to vertical velocity
            vel->vy += _gravity * dt;
            // Note: Position integration is handled by the CollisionSystem after resolving tile collisions
        }
    }
}

void PhysicsSystem::set_gravity(float g) { _gravity = g; }

} // namespace mario
