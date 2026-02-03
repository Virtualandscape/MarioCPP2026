#include "mario/systems/PhysicsSystem.hpp"

#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"

namespace mario {

void PhysicsSystem::update(EntityManager& registry, float dt) const
{
    auto entities = registry.get_entities_with<PositionComponent>();
    for (auto entity : entities) {
        auto* pos = registry.get_component<PositionComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);
        if (pos && vel) {
            vel->vy += _gravity * dt;
            pos->x += vel->vx * dt;
            pos->y += vel->vy * dt;
        }
    }
}

void PhysicsSystem::set_gravity(float g) { _gravity = g; }

} // namespace mario
