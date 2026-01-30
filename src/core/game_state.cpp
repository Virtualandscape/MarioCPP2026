#include "mario/core/GameState.hpp"

namespace mario {
    void PlayState::on_enter() {
        _player.set_position(32.0f, 32.0f);
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
    }

    void PlayState::render() {
        _renderer.begin_frame();
        _renderer.draw_ellipse(_player.x(), _player.y(), _player.width(), _player.height());
        _renderer.end_frame();
    }

    bool PlayState::is_running() const { return _renderer.is_open(); }

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
