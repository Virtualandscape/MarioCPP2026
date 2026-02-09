// Implements the PlayerMovementSystem, which updates player movement and jumping based on input and player stats.
// Handles horizontal movement and double-jump logic for player entities.

#include "mario/systems/PlayerMovementSystem.hpp"

#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/helpers/Constants.hpp"

namespace mario {

void PlayerMovementSystem::update(EntityManager& registry, float dt) const
{
    static thread_local std::vector<EntityID> entities;
    // Query entities that have all required components in one call.
    registry.get_entities_with_all<PlayerInputComponent, VelocityComponent>(entities);

    for (const auto entity : entities) {
        // Components must exist for each returned entity, so direct access is safe.
        auto* input = registry.get_component<PlayerInputComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);

        // Small defensive check in case of unexpected state (keeps behavior robust)
        if (!input || !vel) continue;

        // Handle horizontal movement based on input axis
        vel->vx = input->move_axis * mario::constants::PLAYER_MOVE_SPEED;

        // Handle jumping: allow jump only if not held and jump count below limit (double-jump)
        if (input->jump_pressed && !input->jump_held && input->jump_count < 2) {
            // Compute jump speed to reach PLAYER_JUMP_TILES using the project's gravity setting.
            vel->vy = -mario::constants::jump_speed_for_tiles(mario::constants::PLAYER_JUMP_TILES);
            input->jump_count++;
        }

        // Update jump held state for next frame
        input->jump_held = input->jump_pressed;
    }
}

} // namespace mario
