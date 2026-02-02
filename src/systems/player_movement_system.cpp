#include "mario/systems/PlayerMovementSystem.hpp"

#include "mario/ecs/components/PlayerInput.hpp"
#include "mario/ecs/components/Velocity.hpp"
#include "mario/ecs/components/JumpState.hpp"
#include "mario/ecs/components/PlayerStats.hpp"

namespace mario {

void PlayerMovementSystem::update(Registry& registry, float dt) const
{
    auto entities = registry.get_entities_with<PlayerInput>();
    for (auto entity : entities) {
        auto* input = registry.get_component<PlayerInput>(entity);
        auto* vel = registry.get_component<Velocity>(entity);
        auto* jump = registry.get_component<JumpState>(entity);
        auto* stats = registry.get_component<PlayerStats>(entity);
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
