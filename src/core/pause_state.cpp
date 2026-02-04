// Implements the PauseState class, which represents the paused state of the game.
// Provides empty hooks for entering, exiting, updating, and rendering the pause state.

#include "mario/core/PauseState.hpp"

namespace mario {
    // Called when entering the pause state.
    void PauseState::on_enter() {
    }

    // Called when exiting the pause state.
    void PauseState::on_exit() {
    }

    // Updates the pause state (no-op).
    void PauseState::update(float dt) {
        (void) dt;
    }

    // Renders the pause state (no-op).
    void PauseState::render() {
    }
} // namespace mario
