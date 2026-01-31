#pragma once

#include <memory>
#include <string_view>

namespace mario {
    class TileMap;
    class Camera;
    class Renderer;

    // Loads tilemaps, spawns entities, manages checkpoints.
    class Level {
    public:
        void load(std::string_view level_id);

        void unload();

        void update(float dt);

        void render(Renderer& renderer);

        std::shared_ptr<TileMap> tile_map() const;

        std::shared_ptr<Camera> camera() const;

    private:
        std::shared_ptr<TileMap> _tile_map;
        std::shared_ptr<Camera> _camera;
    };
} // namespace mario
