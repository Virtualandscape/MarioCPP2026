#include "mario/systems/PhysicsSystem.hpp"

#include "mario/ecs/components/Position.hpp"
#include "mario/ecs/components/Velocity.hpp"

namespace mario {

void PhysicsSystem::update(Registry& registry, float dt) const
{
    auto entities = registry.get_entities_with<Position>();
    for (auto entity : entities) {
        auto* pos = registry.get_component<Position>(entity);
        auto* vel = registry.get_component<Velocity>(entity);
        if (pos && vel) {
            vel->vy += _gravity * dt;
            pos->x += vel->vx * dt;
            pos->y += vel->vy * dt;
        }
    }
}

void PhysicsSystem::set_gravity(float g) { _gravity = g; }

} // namespace mario
