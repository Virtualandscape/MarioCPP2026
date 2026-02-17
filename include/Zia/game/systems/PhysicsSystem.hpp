#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include "Zia/game/helpers/Constants.hpp"

namespace zia {

    // Integrates movement, gravity, friction.
    class PhysicsSystem {
    public:
        void update(zia::engine::IEntityManager& registry, float dt) const;

        void set_gravity(float g);

    private:
        // Gravity is in pixels/sec^2; scale it with TILE_SCALE so gameplay in tiles remains consistent when changing tile size.
        float _gravity = 1200.0f * zia::constants::TILE_SCALE;
    };
} // namespace Zia
