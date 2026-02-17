#pragma once

#include "mario/engine/EntityManagerFacade.hpp"
#include "mario/engine/IAssetManager.hpp"
#include <string>

namespace mario {
    class Level;
    struct EntitySpawn;

    class Spawner {
    public:
        static EntityID spawn_player(mario::engine::EntityManagerFacade& registry, const EntitySpawn& spawn, mario::engine::IAssetManager& assets);
        static EntityID spawn_player_default(mario::engine::EntityManagerFacade& registry, mario::engine::IAssetManager& assets);
        static void spawn_enemy(mario::engine::EntityManagerFacade& registry, const EntitySpawn& spawn);
        static void spawn_clouds(mario::engine::EntityManagerFacade& registry, mario::engine::IAssetManager& assets);
    };
} // namespace mario
