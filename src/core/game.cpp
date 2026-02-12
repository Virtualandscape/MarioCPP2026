// Connects the platformer Game implementation to the reusable engine infrastructure.

#include "mario/core/Game.hpp"
#include "mario/core/MenuState.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

#include <memory>

namespace mario {
    // Used by: main.cpp (instantiates mario::Game)
    // Default constructor: behavior is provided by the header/defaulted.
    Game::Game() = default;

    // Used by: main.cpp (destructed when the Game instance goes out of scope)
    // Default destructor: ensure RAII-managed members are destroyed cleanly.
    Game::~Game() = default;

    // Used by: Game::run()
    // Initialize runtime flags and any subsystems that need an explicit start.
    void Game::initialize() {
        // Mark the main loop as running; actual setup is minimal here.
        _running = true;
    }

    // Used by: Game::run(), main.cpp (explicit shutdown)
    // Shutdown the game, releasing or clearing owned resources and states.
    void Game::shutdown() {
        // Clear active states to trigger their destructors and on_exit semantics.
        _states.clear();
        // Unload loaded assets (textures/sounds) from the asset manager.
        _assets.unload_all();
        // Clear the entity manager's storage (ECS reset).
        _entities.clear();
        // Mark as not running to stop any loops.
        _running = false;
    }

    // Used by: main.cpp
    // Run the whole lifecycle: initialize, enter the loop, then shutdown.
    void Game::run() {
        // Prepare runtime
        initialize();
        // Allow derived classes to set up an initial state (push the menu/state).
        before_loop();
        // Enter the fixed-timestep main loop which runs until _running is false.
        main_loop();
        // Perform cleanup after the loop ends.
        shutdown();
    }

    // Used by: MenuState (push PlayState), Game::before_loop()
    // Push a new state onto the stack and call lifecycle hooks.
    void Game::push_state(std::shared_ptr<GameState> state) {
        if (!state) {
            return;
        }
        // If a state is currently active, notify it that it loses focus.
        if (const auto current = current_state()) {
            current->on_exit();
        }
        // Store the new state (shared ownership) and notify it that it becomes active.
        _states.push_back(std::move(state));
        _states.back()->on_enter();
    }

    // Used by: PlayState (to exit the current state)
    // Pop the current state and stop the game if no more states remain.
    void Game::pop_state() {
        if (_states.empty()) {
            return;
        }
        // Notify the top state it is exiting, then remove it.
        _states.back()->on_exit();
        _states.pop_back();
        // If no states remain, the game loop should terminate.
        if (_states.empty()) {
            _running = false;
        }
    }

    // Used by: Game::push_state(), Game::before_loop(), Game::main_loop()
    // Return the currently active state (or nullptr when none).
    std::shared_ptr<GameState> Game::current_state() {
        if (_states.empty()) {
            return nullptr;
        }
        return _states.back();
    }

    // Used by: PlayState, MenuState, rendering systems and HUD
    // Accessor for the renderer owned by Game.
    Renderer &Game::renderer() {
        return _renderer;
    }

    // Used by: PlayState::update(), MenuState::update()
    // Accessor for the input manager owned by Game.
    InputManager &Game::input() {
        return _input;
    }

    // Used by: PlayState::on_enter() (loads textures), render systems
    // Accessor for the asset manager owned by Game.
    AssetManager &Game::assets() {
        return _assets;
    }

    // Used by: PlayState (update/render systems)
    // Accessor for the entity manager (ECS registry) owned by Game.
    EntityManager &Game::entity_manager() {
        return _entities;
    }

    // Used by: Game::run(); can be overridden by derived classes to push an initial state
    // Hook called before entering the main loop: ensure an initial state exists.
    void Game::before_loop() {
        // If no state is present, push the menu state as initial.
        if (!current_state()) {
            push_state(std::make_shared<MenuState>(*this));
        }
    }

    // Used by: Game::run()
    // The main fixed-timestep loop: updates and renders the current state.
    // Cache the current state per-frame and use clock.restart() for clearer dt.
    void Game::main_loop() {
        sf::Clock clock;
        constexpr sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);

        while (_running) {
            const auto state = current_state();
            if (!state) break;

            // Compute delta time in seconds since last frame.
            const float dt = clock.restart().asSeconds();

            state->update(dt);
            state->render();

            const sf::Time elapsed = clock.getElapsedTime();
            if (elapsed < target_frame_time) {
                sf::sleep(target_frame_time - elapsed);
            }
        }
    }
} // namespace mario
