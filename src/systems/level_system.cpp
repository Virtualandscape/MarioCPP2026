#include "mario/systems/LevelSystem.hpp"
#include "mario/world/Level.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/JumpStateComponent.hpp"
#include <cmath>
#include <algorithm>

namespace mario {

void LevelSystem::check_ground_status(Registry& registry, const TileMap& map) {
    auto entities = registry.get_entities_with<JumpStateComponent>();
    const int tile_size = map.tile_size();
    if (tile_size <= 0) return;

    for (auto entity : entities) {
        auto* pos = registry.get_component<PositionComponent>(entity);
        auto* size = registry.get_component<SizeComponent>(entity);
        auto* jump = registry.get_component<JumpStateComponent>(entity);

        if (pos && size && jump) {
            constexpr float epsilon = 0.1f;
            const float bottom = pos->y + size->height;
            const int ty = static_cast<int>(std::floor((bottom + epsilon) / tile_size));
            const int start_tx = static_cast<int>(std::floor(pos->x / tile_size));
            int end_tx = static_cast<int>(std::floor((pos->x + size->width - epsilon) / tile_size));
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

bool LevelSystem::handle_transitions(Registry& registry, EntityID player_id, Level& level, std::string& current_level_path, float& transition_delay, float dt) {
    auto tile_map = level.tile_map();
    if (!tile_map) return false;

    auto* pos = registry.get_component<PositionComponent>(player_id);
    auto* size = registry.get_component<SizeComponent>(player_id);
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
            if (current_level_path.find("level1") != std::string::npos) {
                current_level_path = "assets/levels/level2.json";
            } else {
                current_level_path = "assets/levels/level1.json";
            }
            return true;
        }
    }
    return false;
}

} // namespace mario
