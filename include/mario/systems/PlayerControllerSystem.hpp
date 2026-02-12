#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class InputManager;

    // Coordinates player input, movement, and animation hints in one ECS pass.
    class PlayerControllerSystem {
    public:
        void update(EntityManager& registry, const InputManager& input, float dt) const;
    };
} // namespace mario

