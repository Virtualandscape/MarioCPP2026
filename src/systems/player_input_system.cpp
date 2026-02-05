// Implements the PlayerInputSystem, which updates player input components based on the current input state.
// Maps input actions to movement and jump controls for player entities. This system runs in read mode on InputManager.

#include "mario/systems/PlayerInputSystem.hpp"

#include "mario/input/InputManager.hpp"
#include "mario/ecs/components/PlayerInputComponent.hpp"

namespace mario {
    // Updates player input components for all entities based on the input manager state.
    // Reads current input state and writes to PlayerInputComponent for consumption by other systems.
    void PlayerInputSystem::update(EntityManager &registry, const InputManager &input) const {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PlayerInputComponent>(entities);

        // Cache input state to avoid repeated lookups
        const bool jump_pressed = input.is_pressed(InputManager::Action::Jump);
        const bool move_left = input.is_pressed(InputManager::Action::MoveLeft);
        const bool move_right = input.is_pressed(InputManager::Action::MoveRight);

        // Compute movement axis: -1.0 (left), 0.0 (idle), 1.0 (right)
        const float move_axis = (move_left ? -1.0f : 0.0f) + (move_right ? 1.0f : 0.0f);

        // Update all player input components with current input state
        for (auto entity: entities) {
            auto* player_input = registry.get_component<PlayerInputComponent>(entity);
            if (player_input) {
                // Cache previous jump state (held) before updating current
                player_input->jump_held = player_input->jump_pressed;
                // Update with current frame input
                player_input->jump_pressed = jump_pressed;
                player_input->move_axis = move_axis;
            }
        }
    }
} // namespace mario
