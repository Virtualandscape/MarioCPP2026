#include "mario/world/TileMap.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>

namespace mario {
    namespace {
        void build_default(TileMap& map)
        {
            map.unload();
            map.load({});
        }

        bool extract_int_field(const std::string& text, const char* key, int& value)
        {
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

        std::vector<std::string> extract_string_array(const std::string& text, const char* key)
        {
            std::vector<std::string> result;
            const std::string needle = std::string("\"") + key + "\"";
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

        std::ifstream open_level_file(std::string_view path)
        {
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

            for (const auto& candidate : tries) {
                file = std::ifstream{candidate.string()};
                if (file) {
                    return file;
                }
            }

            return {};
        }
    }

    void TileMap::load(std::string_view map_id) {
        if (map_id.empty()) {
            _width = 50;
            _height = 18;
            _tile_size = 16;
            _tiles.assign(static_cast<std::size_t>(_width * _height), 0);

            for (int x = 0; x < _width; ++x) {
                _tiles[static_cast<std::size_t>((_height - 1) * _width + x)] = 1;
            }

            for (int x = 10; x < 16; ++x) {
                _tiles[static_cast<std::size_t>((_height - 5) * _width + x)] = 1;
            }
            return;
        }

        std::ifstream file = open_level_file(map_id);
        if (!file) {
            build_default(*this);
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        int width = 0;
        int height = 0;
        int tile_size = 16;
        if (!extract_int_field(content, "width", width) ||
            !extract_int_field(content, "height", height)) {
            build_default(*this);
            return;
        }

        extract_int_field(content, "tileSize", tile_size);
        const auto rows = extract_string_array(content, "rows");

        if (width <= 0 || height <= 0 || tile_size <= 0 || rows.empty()) {
            build_default(*this);
            return;
        }

        _width = width;
        _height = height;
        _tile_size = tile_size;
        _tiles.assign(static_cast<std::size_t>(_width * _height), 0);

        const int row_count = std::min(static_cast<int>(rows.size()), _height);
        for (int y = 0; y < row_count; ++y) {
            const std::string& row = rows[static_cast<std::size_t>(y)];
            const int col_count = std::min(static_cast<int>(row.size()), _width);
            for (int x = 0; x < col_count; ++x) {
                if (row[static_cast<std::size_t>(x)] == '1') {
                    _tiles[static_cast<std::size_t>(y * _width + x)] = 1;
                }
            }
        }
    }

    void TileMap::unload() {
        _tiles.clear();
        _width = 0;
        _height = 0;
    }

    void TileMap::update(float dt) { (void) dt; }

    void TileMap::render() {
    }

    int TileMap::width() const { return _width; }

    int TileMap::height() const { return _height; }

    int TileMap::tile_size() const { return _tile_size; }

    bool TileMap::is_solid(int tx, int ty) const {
        if (tx < 0 || ty < 0 || tx >= _width || ty >= _height) {
            return false;
        }

        const std::size_t index = static_cast<std::size_t>(ty * _width + tx);
        return _tiles[index] != 0;
    }

    int TileMap::clamp_tile_x(int tx) const { return std::clamp(tx, 0, std::max(0, _width - 1)); }

    int TileMap::clamp_tile_y(int ty) const { return std::clamp(ty, 0, std::max(0, _height - 1)); }
} // namespace mario
