#pragma once

#include "mario/ecs/EntityManager.hpp"
#include <string>

namespace mario {
    class Level;
    struct EntitySpawn;
    class AssetManager;

    class Spawner {
    public:
        static EntityID spawn_player(EntityManager& registry, const EntitySpawn& spawn, float tile_size);
        static EntityID spawn_player_default(EntityManager& registry);
        static void spawn_enemy(EntityManager& registry, const EntitySpawn& spawn, float tile_size);
        static void spawn_clouds(EntityManager& registry, AssetManager& assets);
    };
} // namespace mario
