#pragma once

#include <string_view>

namespace mario {
    //  Tile grid data, collision layer, rendering chunks.
    class TileMap {
    public:
        void load(std::string_view map_id);

        void unload();

        void update(float dt);

        void render();
    };
} // namespace mario
