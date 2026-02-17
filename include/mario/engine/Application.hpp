#pragma once

// Engine-level application harness. Provides the main loop, managers and scene stack.
// This header intentionally lives under include/mario/engine to keep project includes consistent.

#include "mario/core/Scene.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/core/UIManager.hpp"

#include <memory>
#include <vector>

#include <SFML/System/Clock.hpp>

namespace mario::engine {
    // A small reusable application class that owns global managers and the main loop.
    class Application {
    public:
        // Construct the application. Title is optional and forwarded to the renderer.
        explicit Application(std::string_view title = {});

        // Default destructor: RAII cleans up owned members.
        virtual ~Application();

        // Initialize subsystems (renderer, UI, input, assets, ECS).
        void initialize();

        // Shutdown and release owned resources.
        void shutdown();

        // Run the main loop until no scenes remain.
        void run();

        // Scene stack management.
        void push_scene(std::shared_ptr<mario::Scene> scene);
        void pop_scene();
        std::shared_ptr<mario::Scene> current_scene();

        // Accessors for subsystems (non-owning references returned).
        mario::Renderer &renderer();
        mario::InputManager &input();
        mario::AssetManager &assets();
        mario::EntityManager &entity_manager();

    protected:
        // Hook for derived classes to prepare an initial scene before the loop begins.
        virtual void before_loop();

    private:
        // Core loop implementation (fixed timestep and frame throttling).
        void main_loop();

        // Running flag for the main loop.
        bool _running = false;

        // Owned subsystems. Unique ownership ensures clear RAII semantics.
        std::unique_ptr<mario::Renderer> _renderer;
        std::unique_ptr<mario::InputManager> _input;
        std::unique_ptr<mario::AssetManager> _assets;
        std::unique_ptr<mario::EntityManager> _entities;

        // Active scene stack (shared ownership of scenes keeps interfaces simple).
        std::vector<std::shared_ptr<mario::Scene>> _scenes;

        // Clock used by ImGui/SFML integration.
        sf::Clock _imgui_clock;

        // Centralized UI manager that wraps ImGui-SFML usage.
        mario::UIManager _ui;
    };
} // namespace mario::engine

