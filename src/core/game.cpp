#include "mario/core/Game.hpp"
#include "mario/core/GameState.hpp"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

namespace mario {
    // Redundant for now, but will be useful later.
    void Game::initialize() {
        _running = true;
    }

    // Cleanup resources and set the running flag to false.
    void Game::shutdown() {
        _states.clear();
        _running = false;
    }

    void Game::run() {
        initialize();

        // Initialize the game with a default menu state if none is set.
        if (!current_state()) {
            push_state(std::make_shared<MenuState>(*this));
        }

        sf::Clock clock;
        const sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);
        sf::Time previous_time = clock.getElapsedTime();
        // While the game is running and there is a current state, processes updates and rendering.
        while (_running && current_state()) {
            const sf::Time current_time = clock.getElapsedTime();
            const float dt = (current_time - previous_time).asSeconds();
            previous_time = current_time;

            auto state = current_state();
            state->update(dt);
            state->render();

            const sf::Time work_time = clock.getElapsedTime() - current_time;
            if (work_time < target_frame_time) {
                sf::sleep(target_frame_time - work_time);
            }

            if (!state->is_running()) {
                _running = false;
            }
        }

        shutdown();
    }

    // Push a new game state onto the stack, exiting the current state if any.
    void Game::push_state(std::shared_ptr<GameState> state) {
        if (!state) {
            return;
        }
        // Ensure the current state is exited before pushing a new one.
        if (auto current = current_state()) {
            current->on_exit();
        }
        // Push the new state onto the stack and initialize it.
        _states.push_back(std::move(state));
        // Access the last element of the stack and call on_enter to initialize the new state.
        _states.back()->on_enter();
    }

    void Game::pop_state() {
        if (_states.empty()) {
            return;
        }
        // Ensure the current state is exited before removing it.
        _states.back()->on_exit();
        // Remove the current state from the stack.
        _states.pop_back();

        if (_states.empty()) {
            _running = false;
        }
    }

    // Returns the current game state, or nullptr if no state is active.
    std::shared_ptr<GameState> Game::current_state() {
        if (_states.empty()) {
            return {};
        }

        return _states.back();
    }
} // namespace mario
