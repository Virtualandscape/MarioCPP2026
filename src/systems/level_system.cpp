// Implements the LevelSystem, which checks and updates the ground status for entities (e.g., for jumping logic).
// Determines if entities are on the ground by checking for solid tiles beneath them.

#include "mario/systems/LevelSystem.hpp"
#include "mario/world/Level.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/JumpStateComponent.hpp"
#include "mario/util/Constants.hpp"

#include <algorithm>
#include <string>
#include <cmath>

namespace mario {
    // Checks if entities with JumpStateComponent are on the ground by testing for solid tiles below them.
    void LevelSystem::check_ground_status(EntityManager &registry, const TileMap &map) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<JumpStateComponent>(entities);
        const int tile_size = map.tile_size();
        if (tile_size <= 0) return;

        for (auto entity: entities) {
            auto *pos = registry.get_component<PositionComponent>(entity);
            auto *size = registry.get_component<SizeComponent>(entity);
            auto *jump = registry.get_component<JumpStateComponent>(entity);

            if (pos && size && jump) {
                constexpr float epsilon = 0.1f;
                const float bottom = pos->y + size->height;
                const int ty = static_cast<int>(std::floor((bottom + epsilon) / static_cast<float>(tile_size)));
                const int start_tx = static_cast<int>(std::floor(pos->x / static_cast<float>(tile_size)));
                int end_tx = static_cast<int>(std::floor((pos->x + size->width - epsilon) / static_cast<float>(tile_size)));
                if (end_tx < start_tx) end_tx = start_tx;

                bool on_ground = false;
                for (int tx = start_tx; tx <= end_tx; ++tx) {
                    if (map.is_solid(tx, ty)) {
                        on_ground = true;
                        break;
                    }
                }

                if (on_ground) {
                    jump->jump_count = 0;
                }
            }
        }
    }

    bool LevelSystem::handle_transitions(EntityManager &registry, EntityID player_id, Level &level,
                                         std::string &current_level_path, float &transition_delay, float dt) {
        auto tile_map = level.tile_map();
        if (!tile_map) return false;

        auto *pos = registry.get_component<PositionComponent>(player_id);
        auto *size = registry.get_component<SizeComponent>(player_id);
        if (!pos || !size) return false;

        if (transition_delay > 0.0f) {
            transition_delay = std::max(0.0f, transition_delay - dt);
        }

        // Reset the level if the player falls below the map
        float map_bottom = static_cast<float>(tile_map->height()) * static_cast<float>(tile_map->tile_size());
        if (pos->y > map_bottom) {
            return true;
        }

        if (transition_delay <= 0.0f) {
            const int map_right_px = tile_map->width() * tile_map->tile_size();
            if (pos->x + size->width > static_cast<float>(map_right_px)) {
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
