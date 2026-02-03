#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class TileMap;

    class EnemySystem {
    public:
        void update(EntityManager& registry, const TileMap& map, float dt) const;
    };
} // namespace mario
