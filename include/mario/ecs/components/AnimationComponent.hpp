#pragma once

#include <string>

namespace mario {
    struct AnimationComponent {
        enum class State {
            Idle,
            Run,
            Jump
        };

        State current_state = State::Idle;
        int current_frame = 0;
        float frame_timer = 0.0f;
        float frame_duration = 0.1f;
        int frame_count = 1;
        bool flip_x = true;
        bool is_dirty = true; // To force initial rect update
    };
} // namespace mario
