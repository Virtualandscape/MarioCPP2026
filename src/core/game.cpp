#include "mario/core/Game.hpp"
#include "mario/core/GameState.hpp"

#include <SFML/System/Clock.hpp>

namespace mario {
    void Game::initialize() {
        running_ = true;
    }

    void Game::shutdown() {
        states_.clear();
        running_ = false;
    }

    void Game::run() {
        initialize();

        if (!current_state()) {
            push_state(std::make_shared<PlayState>());
        }

        sf::Clock clock;
        while (running_ && current_state()) {
            const float dt = clock.restart().asSeconds();
            auto state = current_state();
            state->update(dt);
            state->render();

            if (!state->is_running()) {
                running_ = false;
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

        states_.push_back(std::move(state));
        states_.back()->on_enter();
    }

    void Game::pop_state() {
        if (states_.empty()) {
            return;
        }

        states_.back()->on_exit();
        states_.pop_back();

        if (states_.empty()) {
            running_ = false;
        }
    }

    std::shared_ptr<GameState> Game::current_state() {
        if (states_.empty()) {
            return {};
        }

        return states_.back();
    }
} // namespace mario
