#include "mario/systems/PlayerInputSystem.hpp"

#include "mario/input/InputManager.hpp"
#include "mario/ecs/components/PlayerInput.hpp"

namespace mario {
    void PlayerInputSystem::update(Registry &registry, const InputManager &input) const {
        auto entities = registry.get_entities_with<PlayerInput>();
        for (auto entity: entities) {
            auto *player_input = registry.get_component<PlayerInput>(entity);
            if (player_input) {
                player_input->jump_held = player_input->jump_pressed; // previous
                player_input->jump_pressed = input.is_pressed(InputManager::Action::Jump); // current
                player_input->move_axis = 0.0f;
                if (input.is_pressed(InputManager::Action::MoveLeft)) player_input->move_axis -= 1.0f;
                if (input.is_pressed(InputManager::Action::MoveRight)) player_input->move_axis += 1.0f;
            }
        }
    }
} // namespace mario
