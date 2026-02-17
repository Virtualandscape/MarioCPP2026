#pragma once

#include "Zia/engine/IEntityManager.hpp"

namespace zia {
    class TileMap;

    class EnemySystem {
    public:
        void update(zia::engine::IEntityManager& registry, const TileMap& map, float dt) const;
    };
} // namespace Zia
