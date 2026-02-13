// Used by: play_state (loads level file, background), tile_map (helper variant), tests
// Implements the Level class, which manages loading, unloading, updating, and rendering a game level.
// Handles reading level data from files, extracting metadata, and managing camera and tile map state.

#include "mario/world/Level.hpp"
#include "mario/world/Camera.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/render/Renderer.hpp"

#include "mario/world/JsonHelper.hpp"

#include <algorithm>
#include <utility>
#include <filesystem>
#include <fstream>
#include <cmath>

namespace mario {
    // Removed anonymous namespace helpers; they are now in mario::JsonHelper

    // Used by: PlayState::on_enter, PlayState (loads level and sets camera bounds), tests
    // Loads a level from a JSON file, initializes the tile map, entity spawns, background, and camera bounds.
    void Level::load(std::string_view level_id) {
        _tile_map = std::make_shared<TileMap>();
        std::vector<EntitySpawn> spawns;
        _tile_map->load(level_id, std::optional<std::reference_wrapper<std::vector<EntitySpawn>>>(std::ref(spawns))); // Load tile map and collect entity spawn points
        _entity_spawns = std::move(spawns);

        // Try to read the background image path and scale from the level file (if present)
        _background_path.clear();
        _background_scale = 1.0f;
        if (!level_id.empty()) {
            std::ifstream file = JsonHelper::open_level_file(level_id);
            if (file) {
                const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                std::string bg;
                if (JsonHelper::extract_string_field(content, "background", bg)) {
                    _background_path = bg;
                }
                float bscale = 1.0f;
                if (JsonHelper::extract_float_field(content, "background_scale", bscale)) {
                    _background_scale = bscale;
                }

                // Parse background_layers
                _background_layers.clear();
                std::size_t layers_pos = content.find("\"background_layers\"");
                if (layers_pos != std::string::npos) {
                    std::size_t colon_pos = content.find(':', layers_pos);
                    if (colon_pos != std::string::npos) {
                        std::size_t bracket_pos = content.find('[', colon_pos);
                        if (bracket_pos != std::string::npos) {
                            std::size_t end_bracket = content.find(']', bracket_pos);
                            if (end_bracket != std::string::npos) {
                                std::string layers_content = content.substr(bracket_pos + 1, end_bracket - bracket_pos - 1);
                                // Parse the object
                                BackgroundLayer layer;
                                if (JsonHelper::extract_string_field(layers_content, "path", layer.path)) {
                                    JsonHelper::extract_float_field(layers_content, "scale", layer.scale);
                                    JsonHelper::extract_float_field(layers_content, "parallax", layer.parallax);
                                    JsonHelper::extract_bool_field(layers_content, "repeat", layer.repeat);
                                    JsonHelper::extract_bool_field(layers_content, "repeat_x", layer.repeat_x);
                                    _background_layers.push_back(layer);
                                }
                            }
                        }
                    }
                }

                // Parse clouds
                bool clouds = false;
                if (JsonHelper::extract_bool_field(content, "clouds", clouds)) {
                    _clouds_enabled = clouds;
                }
            }
        }

        _camera = std::make_shared<Camera>();

        // Set camera bounds to match the size of the tile map
        const int tile_size = _tile_map->tile_size();
        const auto map_width = static_cast<float>(_tile_map->width() * tile_size);
        const auto map_height = static_cast<float>(_tile_map->height() * tile_size);
        _camera->set_bounds(0.0f, 0.0f, map_width, map_height);
    }

    // Used by: PlayState::on_exit, LevelSystem (cleanup), tests
    // Unloads the level, releasing tile map and camera resources, and clearing entity spawns and background info.
    void Level::unload() {
        if (_tile_map) _tile_map->unload();
        _tile_map.reset();
        _camera.reset();
        _entity_spawns.clear();
        _background_path.clear();
        _background_layers.clear();
    }

    // Used by: PlayState::update (per-frame), Game loop
    // Updates the camera and any level state that depends on time.
    void Level::update(float dt) {
        if (_camera) _camera->update(dt);
    }

    // Used by: PlayState::render, TileMap::render (delegation), tests
    // Renders the visible solid tiles of the level within the camera's viewport.
    void Level::render(Renderer &renderer) {
        if (!_tile_map || !_camera) return;

        const int tile_size = _tile_map->tile_size();
        const auto viewport = renderer.viewport_size();
        const float view_left = _camera->x();
        const float view_top = _camera->y();
        const float view_right = view_left + viewport.x;
        const float view_bottom = view_top + viewport.y;

        // Calculate visible tile range based on camera viewport
        const int max_tx = std::max(0, _tile_map->width() - 1);
        const int max_ty = std::max(0, _tile_map->height() - 1);
        const int min_tx = std::clamp(static_cast<int>(std::floor(view_left / tile_size)), 0, max_tx);
        const int min_ty = std::clamp(static_cast<int>(std::floor(view_top / tile_size)), 0, max_ty);
        const int max_vis_tx = std::clamp(static_cast<int>(std::floor((view_right - 1.0f) / tile_size)), 0, max_tx);
        const int max_vis_ty = std::clamp(static_cast<int>(std::floor((view_bottom - 1.0f) / tile_size)), 0, max_ty);

        // Draw each solid tile in the visible range
        for (int ty = min_ty; ty <= max_vis_ty; ++ty) {
            for (int tx = min_tx; tx <= max_vis_tx; ++tx) {
                if (_tile_map->is_solid(tx, ty)) {
                    renderer.draw_rect(
                        static_cast<float>(tx * tile_size),
                        static_cast<float>(ty * tile_size),
                        static_cast<float>(tile_size),
                        static_cast<float>(tile_size));
                }
            }
        }
    }

    // Used by: LevelSystem, PlayState
    // Returns a shared pointer to the tile map for this level.
    std::shared_ptr<TileMap> Level::tile_map() const { return _tile_map; }
    // Used by: PlayState, systems that need camera reference
    // Returns a shared pointer to the camera for this level.
    std::shared_ptr<Camera> Level::camera() const { return _camera; }
    // Used by: PlayState (spawning entities), LevelSystem
    // Returns a const reference to the vector of entity spawn points for this level.
    const std::vector<EntitySpawn> &Level::entity_spawns() const { return _entity_spawns; }
} // namespace mario
