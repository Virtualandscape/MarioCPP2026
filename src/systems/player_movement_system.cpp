// Implements the PlayerMovementSystem, which updates player movement and jumping based on input and player stats.
// Handles horizontal movement and double-jump logic for player entities.

#include "mario/systems/PlayerMovementSystem.hpp"

#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/JumpStateComponent.hpp"
#include "mario/ecs/components/PlayerStatsComponent.hpp"

namespace mario {

void PlayerMovementSystem::update(EntityManager& registry, float dt) const
{
    static thread_local std::vector<EntityID> entities;
    // Query entities that have all required components in one call.
    registry.get_entities_with_all<PlayerInputComponent, VelocityComponent, JumpStateComponent, PlayerStatsComponent>(entities);

    for (auto entity : entities) {
        // Components must exist for each returned entity, so direct access is safe.
        auto* input = registry.get_component<PlayerInputComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);
        auto* jump = registry.get_component<JumpStateComponent>(entity);
        auto* stats = registry.get_component<PlayerStatsComponent>(entity);

        // Small defensive check in case of unexpected state (keeps behaviour robust)
        if (!input || !vel || !jump || !stats) continue;

        // Handle horizontal movement based on input axis
        vel->vx = input->move_axis * stats->move_speed;

        // Handle jumping: allow jump only if not held and jump count below limit (double-jump)
        if (input->jump_pressed && !input->jump_held && jump->jump_count < 2) {
            vel->vy = -stats->jump_speed;
            jump->jump_count++;
        }

        // Update jump held state for next frame
        input->jump_held = input->jump_pressed;
    }
}

} // namespace mario
