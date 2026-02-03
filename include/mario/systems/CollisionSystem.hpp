#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class TileMap;

    //  Broadphase/narrowphase, tile + entity collisions.
    class CollisionSystem {
    public:
        static void update(EntityManager& registry, const TileMap& map, float dt);
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
