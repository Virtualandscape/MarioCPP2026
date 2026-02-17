#pragma once

#include "mario/engine/IEntityManager.hpp"

namespace mario {
    class TileMap;

    class EnemySystem {
    public:
        void update(mario::engine::IEntityManager& registry, const TileMap& map, float dt) const;
    };
} // namespace mario
