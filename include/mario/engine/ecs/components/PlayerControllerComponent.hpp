#pragma once

#include "mario/engine/ecs/components/AnimationComponent.hpp"

namespace mario {
    // Holds raw input and derived player movement state for ECS systems.
    struct PlayerControllerComponent {
        // Axis value derived from the latest horizontal movement input (-1 left, +1 right).
        float move_axis = 0.0f;
        // Whether the jump key was pressed this frame.
        bool jump_pressed = false;
        // Whether the jump key was held in the previous frame.
        bool jump_held = false;
        // Counts jumps consumed so far to support double-jumping.
        int jump_count = 0;
        // Tracks whether the entity touched the ground during the last collision pass.
        bool on_ground = false;
        // Indicates which animation state the controller decided for this frame.
        AnimationComponent::State requested_state = AnimationComponent::State::Idle;
        // True when the player should face right, false when facing left.
        bool facing_right = true;
    };
} // namespace mario

