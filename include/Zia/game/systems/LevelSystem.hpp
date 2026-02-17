#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include <string>

namespace zia {
    class Level;
    class TileMap;

    class LevelSystem {
    public:
        static bool handle_transitions(zia::engine::IEntityManager& registry, EntityID player_id, Level& level, std::string& current_level_path, float& transition_delay, float dt);
    };
} // namespace Zia
