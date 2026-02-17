// Implements the PhysicsSystem, which applies gravity and integrates velocity into position for all entities.
// Handles basic physics integration for movement and gravitational acceleration.

#include "Zia/game/systems/PhysicsSystem.hpp"

#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"

namespace zia {

void PhysicsSystem::update(zia::engine::IEntityManager& registry, float dt) const
{
    // Query entities that have both Position and Velocity components
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<PositionComponent, VelocityComponent>(entities);
    for (auto entity : entities) {
        auto pos_opt = registry.get_component<PositionComponent>(entity);
        auto vel_opt = registry.get_component<VelocityComponent>(entity);
        if (pos_opt && vel_opt) {
            auto& pos = pos_opt->get();
            auto& vel = vel_opt->get();
            // Apply gravity acceleration to vertical velocity
            vel.vy += _gravity * dt;
            // Note: Position integration is handled by the CollisionSystem after resolving tile collisions
        }
    }
}

void PhysicsSystem::set_gravity(float g) { _gravity = g; }

} // namespace Zia
