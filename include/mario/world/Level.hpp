#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "mario/world/EntitySpawn.hpp"

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

        const std::vector<EntitySpawn>& entity_spawns() const;

    private:
        std::shared_ptr<TileMap> _tile_map;
        std::shared_ptr<Camera> _camera;
        std::vector<EntitySpawn> _entity_spawns;
    };
} // namespace mario
