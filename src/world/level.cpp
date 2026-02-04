// Implements the Level class, which manages loading, unloading, updating, and rendering a game level.
// Handles reading level data from files, extracting metadata, and managing camera and tile map state.

#include "mario/world/Level.hpp"
#include "mario/world/Camera.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/render/Renderer.hpp"

#include <algorithm>
#include <utility>
#include <filesystem>
#include <fstream>

namespace mario {
    namespace {
        // Attempts to open a level file by searching several relative locations.
        // Returns an ifstream to the first found file, or an empty stream if not found.
        std::ifstream open_level_file(std::string_view path) {
            std::filesystem::path base(path);
            std::ifstream file{base.string()};
            if (file) {
                return file;
            }

            const std::filesystem::path cwd = std::filesystem::current_path();
            const std::filesystem::path tries[] = {
                cwd / base,
                cwd / ".." / base,
                cwd / ".." / ".." / base,
                cwd / ".." / ".." / ".." / base,
            };

            for (const auto &candidate: tries) {
                file = std::ifstream{candidate.string()};
                if (file) {
                    return file;
                }
            }

            return {};
        }

        // Extracts a string field from a JSON-like text by key.
        // Returns true if the field is found and sets 'value'.
        bool extract_string_field(const std::string &text, const char *key, std::string &value) {
            const std::string needle = std::string("\"") + key + "\"";
            std::size_t pos = text.find(needle);
            if (pos == std::string::npos) {
                return false;
            }

            pos = text.find(':', pos);
            if (pos == std::string::npos) {
                return false;
            }

            ++pos;
            while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
                ++pos;
            }

            if (pos >= text.size() || text[pos] != '"') {
                return false;
            }

            const std::size_t start = pos + 1;
            const std::size_t end = text.find('"', start);
            if (end == std::string::npos) {
                return false;
            }

            value = text.substr(start, end - start);
            return true;
        }

        // Extracts a float field from a JSON-like text by key.
        // Returns true if the field is found and sets 'value'.
        bool extract_float_field(const std::string &text, const char *key, float &value) {
            const std::string needle = std::string("\"") + key + "\"";
            std::size_t pos = text.find(needle);
            if (pos == std::string::npos) {
                return false;
            }

            pos = text.find(':', pos);
            if (pos == std::string::npos) {
                return false;
            }

            ++pos;
            while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
                ++pos;
            }

            // Read until non-number char
            const std::size_t start = pos;
            std::size_t end = start;
            // allow digits, decimal point, sign, exponent
            while (end < text.size()) {
                const char c = text[end];
                if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') {
                    ++end;
                    continue;
                }
                break;
            }
            if (end == start) return false;

            try {
                value = std::stof(text.substr(start, end - start));
            } catch (...) {
                return false;
            }
            return true;
        }
    }

    // Loads a level from a JSON file, initializes the tile map, entity spawns, background, and camera bounds.
    void Level::load(std::string_view level_id) {
        _tile_map = std::make_shared<TileMap>();
        std::vector<EntitySpawn> spawns;
        _tile_map->load(level_id, &spawns); // Load tile map and collect entity spawn points
        _entity_spawns = std::move(spawns);

        // Try to read background image path and scale from the level file (if present)
        _background_path.clear();
        _background_scale = 1.0f;
        if (!level_id.empty()) {
            std::ifstream file = open_level_file(level_id);
            if (file) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                std::string bg;
                if (extract_string_field(content, "background", bg)) {
                    _background_path = bg;
                }
                float bscale = 1.0f;
                if (extract_float_field(content, "background_scale", bscale)) {
                    _background_scale = bscale;
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

    // Unloads the level, releasing tile map and camera resources, and clearing entity spawns and background info.
    void Level::unload() {
        if (_tile_map) _tile_map->unload();
        _tile_map.reset();
        _camera.reset();
        _entity_spawns.clear();
        _background_path.clear();
    }

    // Updates the camera and any level state that depends on time.
    void Level::update(float dt) {
        if (_camera) _camera->update(dt);
    }

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
        const int min_tx = std::clamp(static_cast<int>(view_left / tile_size), 0, max_tx);
        const int min_ty = std::clamp(static_cast<int>(view_top / tile_size), 0, max_ty);
        const int max_vis_tx = std::clamp(static_cast<int>((view_right - 1.0f) / tile_size), 0, max_tx);
        const int max_vis_ty = std::clamp(static_cast<int>((view_bottom - 1.0f) / tile_size), 0, max_ty);

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

    // Returns a shared pointer to the tile map for this level.
    std::shared_ptr<TileMap> Level::tile_map() const { return _tile_map; }
    // Returns a shared pointer to the camera for this level.
    std::shared_ptr<Camera> Level::camera() const { return _camera; }
    // Returns a const reference to the vector of entity spawn points for this level.
    const std::vector<EntitySpawn> &Level::entity_spawns() const { return _entity_spawns; }
} // namespace mario
