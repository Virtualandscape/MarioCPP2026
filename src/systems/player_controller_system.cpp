#include "mario/systems/PlayerControllerSystem.hpp"

#include <cmath>

#include "mario/input/InputManager.hpp"
#include "mario/ecs/components/PlayerControllerComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/helpers/Constants.hpp"

namespace mario {

void PlayerControllerSystem::update(mario::engine::EntityManagerFacade& registry, const mario::engine::IInput& input, float /*dt*/) const {
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with_all<PlayerControllerComponent, VelocityComponent>(entities);

    const bool jump_pressed = input.is_pressed(mario::InputManager::Action::Jump);
    const float move_axis = (input.is_pressed(mario::InputManager::Action::MoveRight) ? 1.0f : 0.0f) -
                            (input.is_pressed(mario::InputManager::Action::MoveLeft) ? 1.0f : 0.0f);

    for (const auto entity : entities) {
        auto controller_opt = registry.get_component<PlayerControllerComponent>(entity);
        auto velocity_opt = registry.get_component<VelocityComponent>(entity);
        if (!controller_opt || !velocity_opt) continue;

        auto& controller = controller_opt->get();
        auto& velocity = velocity_opt->get();

        // Keep track of the previous frame so we can detect jump press edges.
        controller.jump_held = controller.jump_pressed;
        controller.jump_pressed = jump_pressed;
        controller.move_axis = move_axis;

        // Horizontal movement drives the physics velocity directly.
        velocity.vx = controller.move_axis * constants::PLAYER_MOVE_SPEED;

        // Jump when the action is newly pressed and we have jumps remaining.
        if (controller.jump_pressed && !controller.jump_held && controller.jump_count < 2) {
            velocity.vy = -constants::jump_speed_for_tiles(constants::PLAYER_JUMP_TILES);
            ++controller.jump_count;
        }

        const float axis_threshold = 0.1f;
        const float horizontal_input = std::abs(controller.move_axis);

        // Determine the facing direction based on movement intent.
        if (horizontal_input > axis_threshold) {
            controller.facing_right = controller.move_axis > 0.0f;
        }

        // Decide the animation state the controller expects for this frame.
        if (!controller.on_ground && controller.jump_count > 0) {
            controller.requested_state = AnimationComponent::State::Jump;
        } else if (horizontal_input > axis_threshold) {
            controller.requested_state = AnimationComponent::State::Run;
        } else {
            controller.requested_state = AnimationComponent::State::Idle;
        }
    }
}

} // namespace mario

