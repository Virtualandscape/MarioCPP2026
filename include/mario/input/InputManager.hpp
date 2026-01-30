#pragma once

#include <array>
#include <cstddef>

namespace mario {
    // Key bindings, edge detection (pressed/released).
    class InputManager {
    public:
        enum class Action {
            MoveLeft,
            MoveRight,
            Jump,
            Count
        };

        void poll();

        bool is_pressed(Action action) const;

        void set_action_state(Action action, bool pressed);

    private:
        std::array<bool, static_cast<std::size_t>(Action::Count)> _pressed = {};
    };
} // namespace mario
