#pragma once

#include "mario/ecs/Registry.hpp"
#include <string>

namespace mario {
    class Level;
    class TileMap;

    class LevelSystem {
    public:
        static void check_ground_status(Registry& registry, const TileMap& map);
        static bool handle_transitions(Registry& registry, EntityID player_id, Level& level, std::string& current_level_path, float& transition_delay, float dt);
    };
} // namespace mario
