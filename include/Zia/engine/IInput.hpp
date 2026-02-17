#pragma once

#include "Zia/engine/input/InputManager.hpp"

namespace zia::engine {
    class IInput {
    public:
        virtual ~IInput() = default;

        // Poll pending input events (typically called once per frame).
        virtual void poll() = 0;

        // Query whether an action is currently pressed.
        virtual bool is_pressed(zia::InputManager::Action action) const = 0;
    };
} // namespace Zia::engine

