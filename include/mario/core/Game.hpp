#pragma once

#include "mario/core/Scene.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/core/UIManager.hpp"
#include "mario/engine/Application.hpp" // Use the engine-level application as backend

#include <memory>
#include <vector>

#include <SFML/System/Clock.hpp>

namespace mario {
    // Core application harness that owns the loop, managers, and active scene stack.
    // This Game class is now a thin wrapper that forwards to an engine::Application instance.
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
        // The previous implementation lived here; we now forward to an engine::Application.
        // The engine application is owned via unique_ptr to enforce single ownership.
        std::unique_ptr<engine::Application> _app;
    };
} // namespace mario
