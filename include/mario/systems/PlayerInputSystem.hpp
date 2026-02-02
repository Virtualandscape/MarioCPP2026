#pragma once

#include "mario/ecs/Registry.hpp"

namespace mario {
    class InputManager;

    class PlayerInputSystem {
    public:
        void update(Registry& registry, const InputManager& input) const;
    };
} // namespace mario
