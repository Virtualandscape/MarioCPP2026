#pragma once

namespace mario {
    class Entity;

    // Integrates movement, gravity, friction.
    class PhysicsSystem {
    public:
        void update(Entity& entity, float dt) const;

        void set_gravity(float g);

    private:
        float _gravity = 1200.0f;
    };
} // namespace mario
