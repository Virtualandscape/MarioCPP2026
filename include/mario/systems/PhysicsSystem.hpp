#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {

    // Integrates movement, gravity, friction.
    class PhysicsSystem {
    public:
        void update(EntityManager& registry, float dt) const;

        void set_gravity(float g);

    private:
        float _gravity = 1200.0f;
    };
} // namespace mario
