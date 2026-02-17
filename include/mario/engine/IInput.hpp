#pragma once

#include "mario/input/InputManager.hpp"

namespace mario::engine {
    class IInput {
    public:
        virtual ~IInput() = default;

        // Poll pending input events (typically called once per frame).
        virtual void poll() = 0;

        // Query whether an action is currently pressed.
        virtual bool is_pressed(mario::InputManager::Action action) const = 0;
    };
} // namespace mario::engine

