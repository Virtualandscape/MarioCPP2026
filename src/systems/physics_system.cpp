#include "mario/systems/PhysicsSystem.hpp"

#include "mario/entities/Entity.hpp"

namespace mario {

void PhysicsSystem::update(Entity& entity, float dt) const
{
    entity.set_velocity(entity.vx(), entity.vy() + _gravity * dt);
    entity.update_position(dt);
}

void PhysicsSystem::set_gravity(float g) { _gravity = g; }

} // namespace mario
