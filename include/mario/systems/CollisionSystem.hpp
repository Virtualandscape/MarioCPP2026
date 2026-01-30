#pragma once

namespace mario {
    //  Broadphase/narrowphase, tile + entity collisions.
    class CollisionSystem {
    public:
        void update();
    };

    //  shapes + collision flags (solid, trigger).
    class Collider {
    public:
        void set_solid(bool solid);
    };

    class Hitbox {
    public:
        void set_size(float w, float h);
    };
} // namespace mario
