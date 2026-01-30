#pragma once

namespace mario {
    class Entity;
    class TileMap;

    //  Broadphase/narrowphase, tile + entity collisions.
    class CollisionSystem {
    public:
        void check_entity_collision(Entity& entity, const TileMap& map, float dt) const;
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
