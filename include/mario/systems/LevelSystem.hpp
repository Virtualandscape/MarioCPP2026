#pragma once

#include "mario/engine/EntityManagerFacade.hpp"
#include <string>

namespace mario {
    class Level;
    class TileMap;

    class LevelSystem {
    public:
        static bool handle_transitions(mario::engine::EntityManagerFacade& registry, EntityID player_id, Level& level, std::string& current_level_path, float& transition_delay, float dt);
    };
} // namespace mario
