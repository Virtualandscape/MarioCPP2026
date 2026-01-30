#pragma once

namespace mario {

    // Integrates movement, gravity, friction.
    class PhysicsSystem {
    public:
        void update(float dt);

        void set_gravity(float g);
    };
} // namespace mario
