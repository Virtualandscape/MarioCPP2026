#include "mario/core/GameState.hpp"

#include <algorithm>

namespace mario {
    void PlayState::on_enter() {
        // Initialize player position
        _player.set_position(32.0f, 32.0f);
        // Tile map initialization
        _tile_map.load("assets/levels/demo.json");
        const int tile_size = _tile_map.tile_size();
        const auto map_width = static_cast<float>(_tile_map.width() * tile_size);
        const auto map_height = static_cast<float>(_tile_map.height() * tile_size);
        // Initialize camera with map bounds
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
        if (_input.is_pressed(InputManager::Action::MoveLeft)) axis -= 1.0f;
        if (_input.is_pressed(InputManager::Action::MoveRight)) axis += 1.0f;

        // Set move axis i.e., direction of movement
        _player.set_move_axis(axis);
        // Check if the player is jumping and the jump button is pressed
        _player.set_jump_pressed(_input.is_pressed(InputManager::Action::Jump));
        // Player velocity update and double jump handling
        _player.handle_input();
        _physics.update(_player, dt);
        _collision.check_entity_collision(_player, _tile_map, dt);
        // Update player and entities and physics
        for (auto &entity: _entities) {
            entity->update(dt);
            _physics.update(*entity, dt);
            _collision.check_entity_collision(*entity, _tile_map, dt);
        }
        // Reset jump if player is on ground
        if (_player.is_on_ground(_tile_map)) _player.reset_jump();

        // Handle camera movement and target for scrolling effect
        const auto viewport = _renderer.viewport_size();
        _camera.set_viewport(viewport.x, viewport.y);
        _camera.set_target(_player.x() + _player.width() * 0.5f,
                           _player.y() + _player.height() * 0.5f);
        _camera.update(dt);
        // Exit the game if the escape key is pressed
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

        _player.render(_renderer);

        for (auto &entity: _entities) {
            entity->render(_renderer);
        }

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
