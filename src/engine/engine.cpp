#include "mario/engine/Engine.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>


namespace mario::engine {
    // Implements the generic engine loop so platform-specific games can reuse the logic.
    void Engine::initialize() {
        _running = true;
    }

    // Cleans up states and cached assets, allowing the engine to be started again safely.
    void Engine::shutdown() {
        _states.clear();
        _scheduler.clear();
        _assets.unload_all();
        _registry.clear();
        _running = false;
    }

    // Prepares, runs, and shuts down the main loop while giving derived classes a hook.
    void Engine::run() {
        initialize();
        before_loop();
        main_loop();
        shutdown();
    }

    // Push a new state and ensure transitions call the expected lifecycle callbacks.
    void Engine::push_state(std::shared_ptr<EngineState> state) {
        if (!state) {
            return;
        }
        if (const auto current = current_state()) {
            current->on_exit();
        }
        _states.push_back(std::move(state));
        _states.back()->on_enter();
    }

    // Pop the current state and stop the engine if the stack becomes empty.
    void Engine::pop_state() {
        if (_states.empty()) {
            return;
        }
        _states.back()->on_exit();
        _states.pop_back();
        if (_states.empty()) {
            _running = false;
        }
    }

    std::shared_ptr<EngineState> Engine::current_state() {
        if (_states.empty()) return nullptr;
        return _states.back();
    }

    Renderer &Engine::renderer() {
        return _renderer;
    }

    InputManager &Engine::input() {
        return _input;
    }

    AssetManager &Engine::assets() {
        return _assets;
    }

    EntityManager &Engine::entity_manager() {
        return _registry;
    }

    SystemScheduler &Engine::system_scheduler() {
        return _scheduler;
    }

    // The default hook does nothing, but derived engines can prepare startup states.
    void Engine::before_loop() {
    }

    // Drives the fixed timestep loop and throttles the frame rate via sleeps.
    void Engine::main_loop() {
        sf::Clock clock;
        constexpr sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);
        sf::Time previous_time = clock.getElapsedTime();

        while (_running && current_state()) {
            const sf::Time current_time = clock.getElapsedTime();
            const float dt = (current_time - previous_time).asSeconds();
            previous_time = current_time;

            const auto state = current_state();
            state->update(dt);
            state->render();

            const sf::Time elapsed = clock.getElapsedTime() - current_time;
            if (elapsed < target_frame_time) {
                sf::sleep(target_frame_time - elapsed);
            }
        }
    }
} // namespace mario::engine

