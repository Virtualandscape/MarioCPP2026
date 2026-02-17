#pragma once

#include "mario/engine/EntityManagerFacade.hpp"

namespace mario {
    class TileMap;

    class EnemySystem {
    public:
        void update(mario::engine::EntityManagerFacade& registry, const TileMap& map, float dt) const;
    };
} // namespace mario
