// Implements the PauseScene class, which represents the paused scene of the game.
// Provides empty hooks for entering, exiting, updating, and rendering the pause scene.

#include "mario/game/PauseScene.hpp"

namespace mario {
    // Called when entering the pause scene.
    void PauseScene::on_enter() {
    }

    // Called when exiting the pause scene.
    void PauseScene::on_exit() {
    }

    // Updates the pause scene (no-op).
    void PauseScene::update(float dt) {
        (void) dt;
    }

    // Renders the pause scene (no-op).
    void PauseScene::render() {
    }
} // namespace mario

