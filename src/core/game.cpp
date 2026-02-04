// Implements the Game class, which manages the main game loop, state transitions, and initialization/shutdown logic.
// Handles running the game, pushing and popping game states, and processing updates and rendering.

#include "mario/core/Game.hpp"
#include "mario/core/GameState.hpp"
#include "mario/core/MenuState.hpp"
#include "mario/core/PlayState.hpp"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

namespace mario {
    // Initializes the game and sets the running flag.
    void Game::initialize() {
        this->_running = true;
    }

    // Cleanup resources and set the running flag to false.
    void Game::shutdown() {
        _states.clear();
        _running = false;
    }

    // Main game loop: initializes, runs, and processes updates and rendering for the current state.
    void Game::run() {
        initialize();

        // Initialize the game with a default menu state if none is set.
        if (!current_state()) {
            push_state(std::make_shared<MenuState>(*this));
        }

        sf::Clock clock;
        constexpr sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);
        sf::Time previous_time = clock.getElapsedTime();
        // While the game is running and there is a current state, processes updates and rendering.
        while (_running && current_state()) {
            const sf::Time current_time = clock.getElapsedTime();
            const float dt = (current_time - previous_time).asSeconds();
            previous_time = current_time;

            const auto state = current_state();
            state->update(dt);
            state->render();

            // Sleep to maintain the target frame rate.
            const sf::Time elapsed = clock.getElapsedTime() - current_time;
            if (elapsed < target_frame_time) {
                sf::sleep(target_frame_time - elapsed);
            }
        }
        shutdown();
    }

    // Pushes a new game state onto the stack.
    void Game::push_state(std::shared_ptr<GameState> state) {
        if (!state) {
            return;
        }
        // Ensure the current state is exited before pushing a new one.
        if (const auto current = current_state()) {
            current->on_exit();
        }
        // Push the new state onto the stack and initialize it.
        _states.push_back(std::move(state));
        // Access the last element of the stack and call on_enter to initialize the new state.
        _states.back()->on_enter();
    }

    // Pops the current game state from the stack.
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

    // Returns the current game state, or nullptr if none.
    std::shared_ptr<GameState> Game::current_state() {
        if (_states.empty()) return nullptr;
        return _states.back();
    }
} // namespace mario
