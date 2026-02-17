#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include "Zia/engine/IAssetManager.hpp"
#include <string>

namespace zia {
    class Level;
    struct EntitySpawn;

    class Spawner {
    public:
        static EntityID spawn_player(zia::engine::IEntityManager& registry, const EntitySpawn& spawn, zia::engine::IAssetManager& assets);
        static EntityID spawn_player_default(zia::engine::IEntityManager& registry, zia::engine::IAssetManager& assets);
        static void spawn_enemy(zia::engine::IEntityManager& registry, const EntitySpawn& spawn);
        static void spawn_clouds(zia::engine::IEntityManager& registry, zia::engine::IAssetManager& assets);
    };
} // namespace Zia
