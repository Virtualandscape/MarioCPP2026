#pragma once

namespace mario {
    struct PlayerInputComponent {
        float move_axis = 0.0f;
        bool jump_pressed = false;
        bool jump_held = false;
        // Track how many jumps have been used (reset when grounded).
        int jump_count = 0;
    };
} // namespace mario
