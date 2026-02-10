#pragma once

#include <vector>
#include <string_view>
#include <optional>
#include <functional>

#include "mario/world/EntitySpawn.hpp"

namespace mario {
    //  Tile grid data, collision layer, rendering chunks.
    class TileMap {
    public:
        // Accept an optional reference to a vector to collect entity spawns (no raw pointer)
        void load(std::string_view map_id, std::optional<std::reference_wrapper<std::vector<EntitySpawn>>> entity_spawns = std::nullopt);

        void unload();

        void update(float dt);

        void render();

        int width() const;

        int height() const;

        int tile_size() const;

        bool is_solid(int tx, int ty) const;

        int clamp_tile_x(int tx) const;

        int clamp_tile_y(int ty) const;

    private:
        int _width = 0;
        int _height = 0;
        // Fixed tile size (pixels). Per-level "tileSize" JSON field is deprecated and ignored.
        int _tile_size = 32;
        std::vector<unsigned char> _tiles;
    };
} // namespace mario
