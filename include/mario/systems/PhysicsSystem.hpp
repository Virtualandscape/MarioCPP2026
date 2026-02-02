#pragma once

#include "mario/ecs/Registry.hpp"

namespace mario {
    class Entity;

    // Integrates movement, gravity, friction.
    class PhysicsSystem {
    public:
        void update(Entity& entity, float dt) const;

        void update(Registry& registry, float dt) const;

        void set_gravity(float g);

    private:
        float _gravity = 1200.0f;
    };
} // namespace mario
