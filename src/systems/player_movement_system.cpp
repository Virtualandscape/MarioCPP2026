#include "mario/systems/PlayerMovementSystem.hpp"

#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/JumpStateComponent.hpp"
#include "mario/ecs/components/PlayerStatsComponent.hpp"

namespace mario {

void PlayerMovementSystem::update(EntityManager& registry, float dt) const
{
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<PlayerInputComponent>(entities);
    for (auto entity : entities) {
        auto* input = registry.get_component<PlayerInputComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);
        auto* jump = registry.get_component<JumpStateComponent>(entity);
        auto* stats = registry.get_component<PlayerStatsComponent>(entity);
        if (input && vel && jump && stats) {
            // Handle movement
            if (input->move_axis != 0.0f) {
                vel->vx = input->move_axis * stats->move_speed;
            } else {
                vel->vx = 0.0f;
            }

            // Handle jumping
            if (input->jump_pressed && !input->jump_held && jump->jump_count < 2) {
                vel->vy = -stats->jump_speed;
                jump->jump_count++;
            }

            // Update jump held
            input->jump_held = input->jump_pressed;
        }
    }
}

} // namespace mario
