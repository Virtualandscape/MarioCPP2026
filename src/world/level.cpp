#include "mario/world/Level.hpp"
#include "mario/world/Camera.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/render/Renderer.hpp"

#include <algorithm>

namespace mario {
    // Loads a level from a JSON file and initializes camera bounds
    void Level::load(std::string_view level_id) {
        _tile_map = std::make_shared<TileMap>();
        _tile_map->load(level_id);

        _camera = std::make_shared<Camera>();
        
        const int tile_size = _tile_map->tile_size();
        const auto map_width = static_cast<float>(_tile_map->width() * tile_size);
        const auto map_height = static_cast<float>(_tile_map->height() * tile_size);
        _camera->set_bounds(0.0f, 0.0f, map_width, map_height);
    }

    // Unloads level resources and resets camera
    void Level::unload() {
        if (_tile_map) _tile_map->unload();
        _tile_map.reset();
        _camera.reset();
    }

    // Updates camera and level state
    void Level::update(float dt) {
        if (_camera) _camera->update(dt);
    }

    // Renders level tiles within the camera viewport
    void Level::render(Renderer& renderer) {
        if (!_tile_map || !_camera) return;

        const int tile_size = _tile_map->tile_size();
        const auto viewport = renderer.viewport_size();
        const float view_left = _camera->x();
        const float view_top = _camera->y();
        const float view_right = view_left + viewport.x;
        const float view_bottom = view_top + viewport.y;

        const int max_tx = std::max(0, _tile_map->width() - 1);
        const int max_ty = std::max(0, _tile_map->height() - 1);
        const int min_tx = std::clamp(static_cast<int>(view_left / tile_size), 0, max_tx);
        const int min_ty = std::clamp(static_cast<int>(view_top / tile_size), 0, max_ty);
        const int max_vis_tx = std::clamp(static_cast<int>((view_right - 1.0f) / tile_size), 0, max_tx);
        const int max_vis_ty = std::clamp(static_cast<int>((view_bottom - 1.0f) / tile_size), 0, max_ty);

        for (int ty = min_ty; ty <= max_vis_ty; ++ty) {
            for (int tx = min_tx; tx <= max_vis_tx; ++tx) {
                if (_tile_map->is_solid(tx, ty)) {
                    renderer.draw_rect(
                        static_cast<float>(tx * tile_size),
                        static_cast<float>(ty * tile_size),
                        static_cast<float>(tile_size),
                        static_cast<float>(tile_size));
                }
            }
        }
    }

    std::shared_ptr<TileMap> Level::tile_map() const { return _tile_map; }
    std::shared_ptr<Camera> Level::camera() const { return _camera; }
} // namespace mario
