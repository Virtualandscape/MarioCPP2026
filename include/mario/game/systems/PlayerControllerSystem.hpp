#pragma once

#include "mario/engine/IEntityManager.hpp"
#include "mario/engine/IInput.hpp"

namespace mario {
    // Handles player input components and applies velocities/forces
    class PlayerControllerSystem {
    public:
        void update(mario::engine::IEntityManager& registry, const mario::engine::IInput& input, float /*dt*/) const;
    };
} // namespace mario

