#include "mario/core/GameState.hpp"

namespace mario {
    void PlayState::on_enter() {
        _tile_map.load("assets/levels/demo.json");
        _player.set_position(32.0f, 32.0f);
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
        _collision.resolve(_player, _tile_map, dt);

        if (_input.is_pressed(InputManager::Action::Escape)) {
            _running = false;
        }
    }

    void PlayState::render() {
        _renderer.begin_frame();
        const int tile_size = _tile_map.tile_size();
        for (int ty = 0; ty < _tile_map.height(); ++ty) {
            for (int tx = 0; tx < _tile_map.width(); ++tx) {
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
