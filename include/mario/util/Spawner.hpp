#pragma once

#include "mario/ecs/Registry.hpp"
#include <string>

namespace mario {
    class Level;
    struct EntitySpawn;

    class Spawner {
    public:
        static EntityID spawn_player(Registry& registry, const EntitySpawn& spawn, float tile_size);
        static EntityID spawn_player_default(Registry& registry);
        static void spawn_enemy(Registry& registry, const EntitySpawn& spawn, float tile_size);
    };
} // namespace mario
