#pragma once

#include <string>

namespace zia {
    // Describes where to spawn an entity (tile coordinates) and what type it should be.
    struct EntitySpawn {
        std::string type;
        int tile_x = 0;
        int tile_y = 0;
        // Optional name provided in level JSON (editor or authoring).
        std::string name;
    };
} // namespace Zia
