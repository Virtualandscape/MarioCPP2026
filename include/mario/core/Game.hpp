#pragma once

#include "mario/core/Scene.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"

#include <memory>
#include <vector>

#include <SFML/System/Clock.hpp>

namespace mario {
    // Core application harness that owns the loop, managers, and active scene stack.
    class Game {
    public:
        Game();

        // Ensures all resources are released when the game object goes out of scope.
        ~Game();

        // Prepares the runtime environment and marks the loop as running.
        void initialize();

        // Gracefully drops all scenes, assets, and ECS data.
        void shutdown();

        // Drives the main loop until no active scenes remain.
        void run();

        // Pushes a new scene onto the stack and triggers its lifecycle hooks.
        void push_scene(std::shared_ptr<Scene> scene);

        // Pops the current scene and stops the loop if no scenes remain.
        void pop_scene();

        // Returns the currently active scene or nullptr when empty.
        std::shared_ptr<Scene> current_scene();

        Renderer &renderer();
        InputManager &input();
        AssetManager &assets();
        EntityManager &entity_manager();

    protected:
        // Hook for derived classes to prepare an initial scene before the loop begins.
        virtual void before_loop();

    private:
        // Implements the fixed-timestep loop and frame throttling.
        void main_loop();

        bool _running = false;
        Renderer _renderer;
        InputManager _input;
        AssetManager _assets;
        EntityManager _entities;
        std::vector<std::shared_ptr<Scene>> _scenes;
        sf::Clock _imgui_clock;
    };
} // namespace mario
