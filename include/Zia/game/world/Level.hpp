#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <string>

#include "Zia/game/world/EntitySpawn.hpp"
#include "Zia/engine/IRenderer.hpp"

namespace zia {
    class TileMap;
    class Camera;

    // Loads tilemaps, spawns entities, manages checkpoints.
    class Level {
    public:
        struct BackgroundLayer {
            std::string path;
            float scale = 1.0f;
            float parallax = 0.0f;
            bool repeat = false;
            bool repeat_x = false;
        };

        void load(std::string_view level_id);

        void unload();

        void update(float dt);

        void render(zia::engine::IRenderer &renderer);

        std::shared_ptr<TileMap> tile_map() const;

        std::shared_ptr<Camera> camera() const;

        const std::vector<EntitySpawn>& entity_spawns() const;

        const std::string& background_path() const { return _background_path; }
        float background_scale() const { return _background_scale; }
        const std::vector<BackgroundLayer>& background_layers() const { return _background_layers; }
        bool clouds_enabled() const { return _clouds_enabled; }

    private:
        std::shared_ptr<TileMap> _tile_map;
        std::shared_ptr<Camera> _camera;
        std::vector<EntitySpawn> _entity_spawns;
        std::string _background_path;
        float _background_scale = 1.0f;
        std::vector<BackgroundLayer> _background_layers;
        bool _clouds_enabled = false;
    };
} // namespace Zia
