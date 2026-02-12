#pragma once

#include "mario/core/GameState.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"

#include <memory>
#include <vector>

namespace mario {
    // Core application harness that owns the loop, managers, and active state stack.
    class Game {
    public:
        Game();

        // Ensures all resources are released when the game object goes out of scope.
        ~Game();

        // Prepares the runtime environment and marks the loop as running.
        void initialize();

        // Gracefully drops all states, assets, and ECS data.
        void shutdown();

        // Drives the main loop until no active states remain.
        void run();

        // Pushes a new state onto the stack and triggers its lifecycle hooks.
        void push_state(std::shared_ptr<GameState> state);

        // Pops the current state and stops the loop if no states remain.
        void pop_state();

        // Returns the currently active state or nullptr when empty.
        std::shared_ptr<GameState> current_state();

        Renderer &renderer();
        InputManager &input();
        AssetManager &assets();
        EntityManager &entity_manager();

    protected:
        // Hook for derived classes to prepare an initial state before the loop begins.
        virtual void before_loop();

    private:
        // Implements the fixed-timestep loop and frame throttling.
        void main_loop();

        bool _running = false;
        Renderer _renderer;
        InputManager _input;
        AssetManager _assets;
        EntityManager _entities;
        std::vector<std::shared_ptr<GameState>> _states;
    };
} // namespace mario
