#pragma once

namespace mario {
    struct PlayerInputComponent {
        float move_axis = 0.0f;
        bool jump_pressed = false;
        bool jump_held = false;
    };
} // namespace mario
