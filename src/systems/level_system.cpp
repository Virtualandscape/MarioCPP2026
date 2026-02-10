// Implements the LevelSystem, which checks and updates the ground status for entities (e.g., for jumping logic).
// Determines if entities are on the ground by checking for solid tiles beneath them.

#include "mario/systems/LevelSystem.hpp"
#include "mario/world/Level.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/helpers/Constants.hpp"

#include <algorithm>
#include <string>
#include <cmath>

namespace mario {
    bool LevelSystem::handle_transitions(EntityManager &registry, EntityID player_id, Level &level,
                                         std::string &current_level_path, float &transition_delay, float dt) {
        auto tile_map = level.tile_map();
        if (!tile_map) return false;

        auto pos_opt = registry.get_component<PositionComponent>(player_id);
        auto size_opt = registry.get_component<SizeComponent>(player_id);
        if (!pos_opt || !size_opt) return false;
        auto& pos = pos_opt->get();
        auto& size = size_opt->get();

        if (transition_delay > 0.0f) {
            transition_delay = std::max(0.0f, transition_delay - dt);
        }

        // Reset the level if the player falls below the map
        float map_bottom = static_cast<float>(tile_map->height()) * static_cast<float>(tile_map->tile_size());
        if (pos.y > map_bottom) {
            return true;
        }

        if (transition_delay <= 0.0f) {
            const int map_right_px = tile_map->width() * tile_map->tile_size();
            if (pos.x + size.width > static_cast<float>(map_right_px)) {
                if (current_level_path == mario::constants::LEVEL1_PATH) {
                    current_level_path = mario::constants::LEVEL2_PATH;
                } else {
                    current_level_path = mario::constants::LEVEL1_PATH;
                }
                return true;
            }
        }
        return false;
    }
} // namespace mario
