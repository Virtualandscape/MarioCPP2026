#pragma once

#include <string>

namespace mario {
    // Describes where to spawn an entity (tile coordinates) and what type it should be.
    struct EntitySpawn {
        std::string type;
        int tile_x = 0;
        int tile_y = 0;
    };
} // namespace mario
