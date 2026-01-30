#pragma once

#include <vector>
#include <string_view>

namespace mario {
    //  Tile grid data, collision layer, rendering chunks.
    class TileMap {
    public:
        void load(std::string_view map_id);

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
        int _tile_size = 16;
        std::vector<unsigned char> _tiles;
    };
} // namespace mario
