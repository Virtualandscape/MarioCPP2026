#include "mario/core/GameState.hpp"

namespace mario {
    void PlayState::on_enter() {
        player_.set_position(32.0f, 32.0f);
    }

    void PlayState::on_exit() {
    }

    void PlayState::update(float dt) {
        input_.poll();

        float axis = 0.0f;
        if (input_.is_pressed(InputManager::Action::MoveLeft)) {
            axis -= 1.0f;
        }
        if (input_.is_pressed(InputManager::Action::MoveRight)) {
            axis += 1.0f;
        }

        player_.set_move_axis(axis);
        player_.set_jump_pressed(input_.is_pressed(InputManager::Action::Jump));
        player_.handle_input();
        player_.update(dt);
    }

    void PlayState::render() {
        renderer_.begin_frame();
        renderer_.draw_ellipse(player_.x(), player_.y(), player_.width(), player_.height());
        renderer_.end_frame();
    }

    bool PlayState::is_running() const { return renderer_.is_open(); }

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
