#pragma once

#include "mario/engine/IEntityManager.hpp"
#include <string>

namespace mario {
    class Level;
    class TileMap;

    class LevelSystem {
    public:
        static bool handle_transitions(mario::engine::IEntityManager& registry, EntityID player_id, Level& level, std::string& current_level_path, float& transition_delay, float dt);
    };
} // namespace mario
