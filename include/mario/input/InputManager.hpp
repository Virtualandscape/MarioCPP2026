#pragma once

namespace mario {
    // Key bindings, edge detection (pressed/released).
    class InputManager {
    public:
        void poll();

        bool is_pressed(int action) const;
    };
} // namespace mario
