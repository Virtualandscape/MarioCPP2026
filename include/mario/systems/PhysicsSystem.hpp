#pragma once

#include "mario/ecs/EntityManager.hpp"
#include "mario/helpers/Constants.hpp"

namespace mario {

    // Integrates movement, gravity, friction.
    class PhysicsSystem {
    public:
        void update(EntityManager& registry, float dt) const;

        void set_gravity(float g);

    private:
        // Gravity is in pixels/sec^2; scale it with TILE_SCALE so gameplay in tiles remains consistent when changing tile size.
        float _gravity = 1200.0f * mario::constants::TILE_SCALE;
    };
} // namespace mario
