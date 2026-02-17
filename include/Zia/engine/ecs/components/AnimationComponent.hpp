#pragma once

#include <string>

namespace zia {
    struct AnimationComponent {
        enum class State {
            Idle,
            Run,
            Jump,
            Celebrate
        };

        State current_state = State::Idle;
        int current_frame = 0;
        float frame_timer = 0.0f;
        float frame_duration = 0.1f;
        int frame_count = 1;
        bool flip_x = true;
        bool needs_rect_update = true; // To force initial rect update
        // If true, this animation will play once and then the system will clear the flag
        bool is_one_shot = false;
        // Number of additional one-shot plays queued (allows playing the same one-shot multiple times)
        int one_shot_queue = 0;
        // Set to true when the animation state was just switched this frame; used to prevent advancing frames immediately.
        bool just_started = false;
    };
} // namespace Zia
