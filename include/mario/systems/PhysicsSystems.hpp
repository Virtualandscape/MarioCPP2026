#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class TileMap;
}

namespace mario::systems::physics {
    // Basic physics integration for movement and gravitational acceleration.
    void update(EntityManager& registry, float dt, float gravity = 980.0f);

    // Broadphase/narrowphase, tile + entity collisions.
    void resolve_collisions(EntityManager& registry, const TileMap& map, float dt);
}

