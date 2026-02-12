#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <functional>

#include "mario/render/Renderer.hpp"

namespace mario {

    // Describes where to spawn an entity (tile coordinates) and what type it should be.
    struct EntitySpawn {
        std::string type;
        int tile_x = 0;
        int tile_y = 0;
    };

    // Tile grid data and collision layer.
    class TileMap {
    public:
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
        int _tile_size = 32;
        std::vector<unsigned char> _tiles;
    };

    // Follows player and handles viewpoint clamping.
    class Camera {
    public:
        void set_target(float x, float y);
        void set_bounds(float left, float top, float right, float bottom);
        void set_viewport(float width, float height);
        void update(float dt);
        [[nodiscard]] float x() const noexcept;
        [[nodiscard]] float y() const noexcept;
        void center_on_target_fraction(float fraction, bool ignore_bounds = false);
        void set_position(float x, float y) noexcept;

    private:
        float _x = 0.0f;
        float _y = 0.0f;
        float _target_x = 0.0f;
        float _target_y = 0.0f;
        float _left = 0.0f, _top = 0.0f, _right = 0.0f, _bottom = 0.0f;
        float _viewport_w = 0.0f, _viewport_h = 0.0f;
        float _follow_speed = 8.0f;
    };

    // Loads tilemaps, spawns entities, and manages level data.
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
        void render(Renderer &renderer);

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

} // namespace mario

