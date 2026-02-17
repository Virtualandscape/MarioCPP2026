#pragma once

#include "Zia/engine/Scene.hpp"
#include "Zia/engine/ecs/EntityManager.hpp"
#include "Zia/engine/input/InputManager.hpp"
#include "Zia/engine/render/Renderer.hpp"
#include "Zia/engine/resources/AssetManager.hpp"
#include "../engine/ui/UIManager.hpp"
#include "Zia/engine/Application.hpp" // Use the engine-level application as backend

#include "Zia/engine/IRenderer.hpp"
#include "Zia/engine/IInput.hpp"
#include "Zia/engine/IAssetManager.hpp"
#include "Zia/engine/IEntityManager.hpp"

#include <memory>
#include <vector>

#include <SFML/System/Clock.hpp>

namespace zia {
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

        // Return engine-level interfaces
        zia::engine::IRenderer &renderer();
        zia::engine::IInput &input();
        zia::engine::IAssetManager &assets();
        zia::engine::IEntityManager &entity_manager();
        // Access to the underlying concrete entity manager if needed by legacy code.
        zia::EntityManager &underlying_entity_manager();

    protected:
        // Hook for derived classes to prepare an initial scene before the loop begins.
        virtual void before_loop();

    private:
        // The previous implementation lived here; we now forward to an engine::Application.
        // The engine application is owned via unique_ptr to enforce single ownership.
        std::unique_ptr<engine::Application> _app;
    };
} // namespace Zia
