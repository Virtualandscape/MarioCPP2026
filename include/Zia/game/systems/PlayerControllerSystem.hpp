#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include "Zia/engine/IInput.hpp"

namespace zia {
    // Handles player input components and applies velocities/forces
    class PlayerControllerSystem {
    public:
        void update(zia::engine::IEntityManager& registry, const zia::engine::IInput& input, float /*dt*/) const;
    };
} // namespace Zia

