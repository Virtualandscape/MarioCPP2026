#include "mario/core/Game.hpp"
#include "mario/core/GameState.hpp"

#include <SFML/System/Clock.hpp>

namespace mario {
    void Game::initialize() {
        _running = true;
    }

    void Game::shutdown() {
        _states.clear();
        _running = false;
    }

    void Game::run() {
        initialize();

        if (!current_state()) {
            push_state(std::make_shared<PlayState>());
        }

        sf::Clock clock;
        while (_running && current_state()) {
            const float dt = clock.restart().asSeconds();
            auto state = current_state();
            state->update(dt);
            state->render();

            if (!state->is_running()) {
                _running = false;
            }
        }

        shutdown();
    }

    void Game::push_state(std::shared_ptr<GameState> state) {
        if (!state) {
            return;
        }

        if (auto current = current_state()) {
            current->on_exit();
        }

        _states.push_back(std::move(state));
        _states.back()->on_enter();
    }

    void Game::pop_state() {
        if (_states.empty()) {
            return;
        }

        _states.back()->on_exit();
        _states.pop_back();

        if (_states.empty()) {
            _running = false;
        }
    }

    std::shared_ptr<GameState> Game::current_state() {
        if (_states.empty()) {
            return {};
        }

        return _states.back();
    }
} // namespace mario
