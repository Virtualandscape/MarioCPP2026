#include "mario/core/GameState.hpp"

#include <algorithm>

namespace mario {
    void PlayState::on_enter() {
        _tile_map.load("assets/levels/demo.json");
        _player.set_position(32.0f, 32.0f);
        const int tile_size = _tile_map.tile_size();
        const float map_width = static_cast<float>(_tile_map.width() * tile_size);
        const float map_height = static_cast<float>(_tile_map.height() * tile_size);
        const auto viewport = _renderer.viewport_size();
        _camera.set_viewport(viewport.x, viewport.y);
        _camera.set_bounds(0.0f, 0.0f, map_width, map_height);
        _camera.set_target(_player.x() + _player.width() * 0.5f,
                           _player.y() + _player.height() * 0.5f);
        _camera.update(0.0f);
        _running = true;
    }

    void PlayState::on_exit() {
    }

    void PlayState::update(float dt) {
        _input.poll();

        float axis = 0.0f;
        if (_input.is_pressed(InputManager::Action::MoveLeft)) {
            axis -= 1.0f;
        }
        if (_input.is_pressed(InputManager::Action::MoveRight)) {
            axis += 1.0f;
        }

        _player.set_move_axis(axis);
        _player.set_jump_pressed(_input.is_pressed(InputManager::Action::Jump));
        _player.handle_input();
        _player.update(dt);
        _physics.update(_player, dt);
        _collision.check_entity_collision(_player, _tile_map, dt);
        if (_player.is_on_ground(_tile_map)) {
            _player.reset_jump();
        }

        const auto viewport = _renderer.viewport_size();
        _camera.set_viewport(viewport.x, viewport.y);
        _camera.set_target(_player.x() + _player.width() * 0.5f,
                           _player.y() + _player.height() * 0.5f);
        _camera.update(dt);

        if (_input.is_pressed(InputManager::Action::Escape)) {
            _running = false;
        }
    }

    void PlayState::render() {
        _renderer.begin_frame();
        _renderer.set_camera(_camera.x(), _camera.y());
        const int tile_size = _tile_map.tile_size();
        const auto viewport = _renderer.viewport_size();
        const float view_left = _camera.x();
        const float view_top = _camera.y();
        const float view_right = view_left + viewport.x;
        const float view_bottom = view_top + viewport.y;

        const int max_tx = std::max(0, _tile_map.width() - 1);
        const int max_ty = std::max(0, _tile_map.height() - 1);
        const int min_tx = std::clamp(static_cast<int>(view_left / tile_size), 0, max_tx);
        const int min_ty = std::clamp(static_cast<int>(view_top / tile_size), 0, max_ty);
        const int max_vis_tx = std::clamp(static_cast<int>((view_right - 1.0f) / tile_size), 0, max_tx);
        const int max_vis_ty = std::clamp(static_cast<int>((view_bottom - 1.0f) / tile_size), 0, max_ty);

        for (int ty = min_ty; ty <= max_vis_ty; ++ty) {
            for (int tx = min_tx; tx <= max_vis_tx; ++tx) {
                if (_tile_map.is_solid(tx, ty)) {
                    _renderer.draw_rect(
                        static_cast<float>(tx * tile_size),
                        static_cast<float>(ty * tile_size),
                        static_cast<float>(tile_size),
                        static_cast<float>(tile_size));
                }
            }
        }

        _renderer.draw_ellipse(_player.x(), _player.y(), _player.width(), _player.height());
        _renderer.end_frame();
    }

    bool PlayState::is_running() const { return _running && _renderer.is_open(); }

    void MenuState::on_enter() {
    }

    void MenuState::on_exit() {
    }

    void MenuState::update(float dt) { (void) dt; }

    void MenuState::render() {
    }

    void PauseState::on_enter() {
    }

    void PauseState::on_exit() {
    }

    void PauseState::update(float dt) { (void) dt; }

    void PauseState::render() {
    }
} // namespace mario
