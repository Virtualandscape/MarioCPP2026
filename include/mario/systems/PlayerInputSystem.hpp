#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class InputManager;

    class PlayerInputSystem {
    public:
        void update(EntityManager& registry, const InputManager& input) const;
    };
} // namespace mario
