#pragma once

#include "mario/ecs/EntityManager.hpp"
#include <string>

namespace mario {
    class Level;
    struct EntitySpawn;
    class AssetManager;

    class Spawner {
    public:
        static EntityID spawn_player(EntityManager& registry, const EntitySpawn& spawn, AssetManager& assets);
        static EntityID spawn_player_default(EntityManager& registry, AssetManager& assets);
        static void spawn_enemy(EntityManager& registry, const EntitySpawn& spawn);
        static void spawn_clouds(EntityManager& registry, AssetManager& assets);
    };
} // namespace mario
