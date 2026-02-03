#pragma once

#include "mario/ecs/EntityManager.hpp"
#include <string>

namespace mario {
    class Level;
    class TileMap;

    class LevelSystem {
    public:
        static void check_ground_status(EntityManager& registry, const TileMap& map);
        static bool handle_transitions(EntityManager& registry, EntityID player_id, Level& level, std::string& current_level_path, float& transition_delay, float dt);
    };
} // namespace mario
