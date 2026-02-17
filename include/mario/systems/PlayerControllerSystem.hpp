#pragma once

#include "mario/engine/EntityManagerFacade.hpp"
#include "mario/engine/IInput.hpp"

namespace mario {
    // Handles player input components and applies velocities/forces
    class PlayerControllerSystem {
    public:
        void update(mario::engine::EntityManagerFacade& registry, const mario::engine::IInput& input, float /*dt*/) const;
    };
} // namespace mario

