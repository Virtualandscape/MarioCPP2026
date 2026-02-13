#include "mario/world/TileMap.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <array>

namespace mario {
    namespace {
        // Fixed the tile size used project-wide (px). JSON "tileSize" is deprecated and ignored.
        constexpr int kFixedTileSize = 32;

        // Loads a default tile map with a single solid tile and entities.
        // Used by: TileMap::load (fallback when file not found or invalid)
        void build_default(TileMap &map, std::optional<std::reference_wrapper<std::vector<EntitySpawn>>> entity_spawns) {
            map.unload();
            map.load({}, entity_spawns);
        }

        // Extract an integer field from a JSON-like string.
        // Used by: TileMap::load (parsing level JSON)
        bool extract_int_field(const std::string &text, std::string_view key, int &value) {
            const std::string needle = std::string("\"") + std::string(key) + "\"";
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

            bool negative = false;
            if (pos < text.size() && text[pos] == '-') {
                negative = true;
                ++pos;
            }

            int result = 0;
            bool found = false;
            while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
                found = true;
                result = (result * 10) + (text[pos] - '0');
                ++pos;
            }

            if (!found) {
                return false;
            }

            value = negative ? -result : result;
            return true;
        }

        // Extract a quoted string field from a JSON-like string.
        // Used by: TileMap::load (parsing level JSON, entity parsing)
        bool extract_string_field(const std::string &text, std::string_view key, std::string &value) {
            const std::string needle = std::string("\"") + std::string(key) + "\"";
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

        bool extract_int_field_any(const std::string &text, const std::array<std::string_view, 2>& keys, int &value) {
            for (const auto &k : keys) {
                if (extract_int_field(text, k, value)) {
                    return true;
                }
            }
            return false;
        }
        // Used by: TileMap::load (parsing player entity spawn)
        bool extract_player_spawn(const std::string &text, EntitySpawn &spawn) {
            const std::string needle = "\"entities\"";
            std::size_t pos = text.find(needle);
            if (pos == std::string::npos) {
                return false;
            }

            pos = text.find('{', pos);
            while (pos != std::string::npos) {
                const std::size_t end = text.find('}', pos);
                if (end == std::string::npos) {
                    break;
                }

                const std::string object = text.substr(pos, end - pos + 1);
                constexpr std::array<std::string_view,2> x_keys = {"x", "tileX"};
                constexpr std::array<std::string_view,2> y_keys = {"y", "tileY"};
                int tile_x = 0;
                int tile_y = 0;
                std::string type;

                if (!extract_string_field(object, "type", type) ||
                    !extract_int_field_any(object, x_keys, tile_x) ||
                    !extract_int_field_any(object, y_keys, tile_y)) {
                    pos = end + 1;
                    continue;
                }

                std::transform(type.begin(), type.end(), type.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });

                if (type != "player") {
                    pos = end + 1;
                    continue;
                }

                spawn.type = "player";
                spawn.tile_x = tile_x;
                spawn.tile_y = tile_y;
                return true;
            }

            return false;
        }

        // Extract an array of strings from a JSON-like string.
        // Used by: TileMap::load (parsing rows array from level JSON)
        std::vector<std::string> extract_string_array(const std::string &text, std::string_view key) {
            std::vector<std::string> result;
            const std::string needle = std::string("\"") + std::string(key) + "\"";
            std::size_t pos = text.find(needle);
            if (pos == std::string::npos) {
                return result;
            }

            pos = text.find('[', pos);
            if (pos == std::string::npos) {
                return result;
            }

            ++pos;
            while (pos < text.size()) {
                if (text[pos] == ']') {
                    break;
                }
                if (text[pos] != '"') {
                    ++pos;
                    continue;
                }

                const std::size_t start = ++pos;
                const std::size_t end = text.find('"', start);
                if (end == std::string::npos) {
                    break;
                }

                result.emplace_back(text.substr(start, end - start));
                pos = end + 1;
            }

            return result;
        }

        // Open a level file from a given path, searching in multiple locations if necessary.
        // Used by: TileMap::load (opening JSON level files)
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
    }

    // Load a tile map from a given identifier, either by ID or by file path.
    // Used by: Level::load (initializes tile map for a level), tests, and any caller that needs to (re)load map data
    void TileMap::load(std::string_view map_id, std::optional<std::reference_wrapper<std::vector<EntitySpawn>>> entity_spawns) {
        if (map_id.empty()) {
            _width = 50;
            _height = 18;
            _tile_size = kFixedTileSize;
            _tiles.assign(static_cast<std::size_t>(_width * _height), 0);

            for (int x = 0; x < _width; ++x) {
                _tiles[static_cast<std::size_t>((_height - 1) * _width + x)] = 1;
            }

            for (int x = 10; x < 16; ++x) {
                _tiles[static_cast<std::size_t>((_height - 5) * _width + x)] = 1;
            }
            if (entity_spawns) {
                entity_spawns->get().clear();
            }
            return;
        }

        std::ifstream file = open_level_file(map_id);

        // If the file cannot be opened, build a default tile map.
        if (!file) {
            build_default(*this, entity_spawns);
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // If there is no width or height, build a default tile map.
        int width = 0;
        int height = 0;
        int tile_size = kFixedTileSize;
        if (!extract_int_field(content, "width", width) ||
            !extract_int_field(content, "height", height)) {
            build_default(*this, entity_spawns);
            return;
        }

        // NOTE: per-level "tileSize" is deprecated and ignored. Use project-wide TILE_SIZE.
        const auto rows = extract_string_array(content, "rows");

        // Warn if JSON contains a tileSize field to help migration
        if (content.find("\"tileSize\"") != std::string::npos) {
            std::cerr << "Warning: level '" << std::string(map_id) << "' contains 'tileSize' which is deprecated and will be ignored; using project TILE_SIZE=" << kFixedTileSize << std::endl;
        }

        // Prefer rows length as a source of truth for map width. If the JSON "width" differs, adopt rows length and warn.
        if (!rows.empty()) {
            const int rows_len = static_cast<int>(rows[0].size());
            if (width != rows_len) {
                std::cerr << "Warning: level '" << std::string(map_id) << "' width field (" << width << ") differs from rows length (" << rows_len << "); using rows length." << std::endl;
                width = rows_len;
            }
        }

        if (entity_spawns) {
            entity_spawns->get().clear();
            EntitySpawn player_spawn;
            if (extract_player_spawn(content, player_spawn)) {
                entity_spawns->get().push_back(player_spawn);
            }
        }

        // If there is no tile size or rows, build a default tile map.
        if (width <= 0 || height <= 0 || tile_size <= 0 || rows.empty()) {
            build_default(*this, entity_spawns);
            return;
        }

        _width = width;
        _height = height;
        _tile_size = tile_size;
        _tiles.assign(static_cast<std::size_t>(_width * _height), 0);

        // Build the tile map from the parsed data.
        const int row_count = std::min(static_cast<int>(rows.size()), _height);
        for (int y = 0; y < row_count; ++y) {
            const std::string &row = rows[static_cast<std::size_t>(y)];
            const int col_count = std::min(static_cast<int>(row.size()), _width);
            for (int x = 0; x < col_count; ++x) {
                char tile_char = row[static_cast<std::size_t>(x)];
                if (tile_char == '1') {
                    _tiles[static_cast<std::size_t>(y * _width + x)] = 1;
                } else if (tile_char == 'G' || tile_char == 'K') {
                    if (entity_spawns) {
                        EntitySpawn spawn;
                        spawn.type = (tile_char == 'G') ? "goomba" : "koopa";
                        spawn.tile_x = x;
                        spawn.tile_y = y;
                        entity_spawns->get().push_back(spawn);
                    }
                }
            }
        }
    }

    // Used by: Level::unload, build_default
    void TileMap::unload() {
        _tiles.clear();
        _width = 0;
        _height = 0;
    }

    // Used by: systems that poll TileMap for time-based behavior; currently not used directly but kept for API consistency
    void TileMap::update(float dt) { (void) dt; }

    // Used by: Level::render (eventual tile rendering). Implementation is currently empty.
    void TileMap::render() {
    }

    // Used by: collision_system.cpp, level.cpp, enemy_system.cpp, helpers/tileSweep.cpp
    int TileMap::width() const { return _width; }

    // Used by: collision_system.cpp, level.cpp, enemy_system.cpp, helpers/tileSweep.cpp
    int TileMap::height() const { return _height; }

    // Used by: collision_system.cpp, level.cpp, enemy_system.cpp, helpers/tileSweep.cpp
    int TileMap::tile_size() const { return _tile_size; }

    // Check if a tile at the given coordinates is solid.
    // Used by: collision_system.cpp, enemy_system.cpp, helpers/tileSweep.cpp, level.cpp
    bool TileMap::is_solid(int tx, int ty) const {
        if (tx < 0 || ty < 0 || tx >= _width || ty >= _height) {
            return false;
        }

        const auto index = static_cast<std::size_t>(ty * _width + tx);
        return _tiles[index] != 0;
    }

    // Used by: various callers that need clamped tile indices (helpers/tileSweep, systems). Provides safe clamping.
    int TileMap::clamp_tile_x(int tx) const { return std::clamp(tx, 0, std::max(0, _width - 1)); }

    // Used by: various callers that need clamped tile indices (helpers/tileSweep, systems). Provides safe clamping.
    int TileMap::clamp_tile_y(int ty) const { return std::clamp(ty, 0, std::max(0, _height - 1)); }
} // namespace mario
