#pragma once

namespace mario {
    struct PlayerInput {
        float move_axis = 0.0f;
        bool jump_pressed = false;
        bool jump_held = false;
    };
} // namespace mario
