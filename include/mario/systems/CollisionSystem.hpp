#pragma once

#include "mario/ecs/Registry.hpp"

namespace mario {
    class TileMap;
    class Entity;

    //  Broadphase/narrowphase, tile + entity collisions.
    class CollisionSystem {
    public:
        void check_entity_collision(Entity& entity, const TileMap& map, float dt) const;
        void check_entity_vs_entity_collision(Entity& a, Entity& b, float dt = 0.0f) const;
        void update(Registry& registry, const TileMap& map, float dt) const;
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
