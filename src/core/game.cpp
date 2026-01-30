#include "mario/core/Game.hpp"
#include "mario/core/GameState.hpp"

namespace mario {
    void Game::initialize() {
    }

    void Game::shutdown() {
    }

    void Game::run() {
        // TODO: initialize subsystems
        initialize();

        // TODO: main loop with update/render
        while (true) {
        }

        shutdown();
    }

    void Game::push_state(std::shared_ptr<GameState> state) { (void) state; }

    void Game::pop_state() {
    }

    std::shared_ptr<GameState> Game::current_state() { return {}; }
} // namespace mario
