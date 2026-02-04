#include "mario/systems/PlayerInputSystem.hpp"

#include "mario/input/InputManager.hpp"
#include "mario/ecs/components/PlayerInputComponent.hpp"

namespace mario {
    void PlayerInputSystem::update(EntityManager &registry, const InputManager &input) const {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PlayerInputComponent>(entities);

        const bool jump_pressed = input.is_pressed(InputManager::Action::Jump);
        const bool move_left = input.is_pressed(InputManager::Action::MoveLeft);
        const bool move_right = input.is_pressed(InputManager::Action::MoveRight);
        const float move_axis = (move_left ? -1.0f : 0.0f) + (move_right ? 1.0f : 0.0f);

        for (auto entity: entities) {
            auto *player_input = registry.get_component<PlayerInputComponent>(entity);
            if (player_input) {
                player_input->jump_held = player_input->jump_pressed; // previous
                player_input->jump_pressed = jump_pressed; // current
                player_input->move_axis = move_axis;
            }
        }
    }
} // namespace mario
