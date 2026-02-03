#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <string>

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

        const std::string& background_path() const { return _background_path; }
        float background_scale() const { return _background_scale; }

    private:
        std::shared_ptr<TileMap> _tile_map;
        std::shared_ptr<Camera> _camera;
        std::vector<EntitySpawn> _entity_spawns;
        std::string _background_path;
        float _background_scale = 1.0f;
    };
} // namespace mario
