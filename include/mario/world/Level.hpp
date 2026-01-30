#pragma once

#include <memory>
#include <string_view>

namespace mario {
    class TileMap;
    class Camera;

    // Loads tilemaps, spawns entities, manages checkpoints.
    class Level {
    public:
        void load(std::string_view level_id);

        void unload();

        void update(float dt);

        void render();

        std::shared_ptr<TileMap> tile_map();

        std::shared_ptr<Camera> camera();
    };
} // namespace mario
