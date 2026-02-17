#pragma once

// Engine-level application harness. Provides the main loop, managers and scene stack.
// This header intentionally lives under include/Zia/engine to keep project includes consistent.

#include "Zia/engine/Scene.hpp"
#include "Zia/engine/IScene.hpp"
#include "Zia/engine/ecs/EntityManager.hpp"
#include "Zia/engine/input/InputManager.hpp"
#include "Zia/engine/render/Renderer.hpp"
#include "Zia/engine/resources/AssetManager.hpp"
#include "Zia/engine/UIManager.hpp"
#include "Zia/engine/IRenderer.hpp"
#include "Zia/engine/IInput.hpp"
#include "Zia/engine/IAssetManager.hpp"
#include "Zia/engine/IEntityManager.hpp"

#include <memory>
#include <vector>

#include <SFML/System/Clock.hpp>

namespace zia::engine {
    // A small reusable application class that owns global managers and the main loop.
    class Application {
    public:
        // Construct the application. Title is optional and forwarded to the renderer.
        explicit Application(std::string_view title = {});

        // Construct the application with injected interface adaptors.
        Application(std::shared_ptr<IRenderer> renderer,
                    std::shared_ptr<IInput> input,
                    std::shared_ptr<IAssetManager> assets,
                    std::shared_ptr<IEntityManager> entities);

        // Default destructor: RAII cleans up owned members.
        virtual ~Application();

        // Initialize subsystems (renderer, UI, input, assets, ECS).
        void initialize();

        // Shutdown and release owned resources.
        void shutdown();

        // Run the main loop until no scenes remain.
        void run();

        // Scene stack management using engine-agnostic IScene pointers.
        void push_scene(std::shared_ptr<IScene> scene);
        // (Note) To push game-specific scenes, wrap them in an engine::adapters::SceneAdapter.
        void pop_scene();
        std::shared_ptr<IScene> current_scene();

        // Accessors for subsystems (non-owning references returned).
        IRenderer &renderer();
        IInput &input();
        IAssetManager &assets();
        IEntityManager &entity_manager();
        // Return the underlying concrete EntityManager for code that depends on concrete APIs.
        zia::EntityManager &underlying_entity_manager();

        // Access to the UI manager.
        UIManager& ui();

    protected:
        // Hook for derived classes to prepare an initial scene before the loop begins.
        virtual void before_loop();

    private:
        // Core loop implementation (fixed timestep and frame throttling).
        void main_loop();

        // Running flag for the main loop.
        bool _running = false;

        // Runtime interface pointers (point to either adapter or default wrapper) used by engine loops.
        std::shared_ptr<IRenderer> _renderer_iface;
        std::shared_ptr<IInput> _input_iface;
        std::shared_ptr<IAssetManager> _assets_iface;
        std::shared_ptr<IEntityManager> _entities_iface;

        // Active scene stack (shared ownership of scenes keeps interfaces simple).
        std::vector<std::shared_ptr<IScene>> _scenes;

        // Clock used by ImGui/SFML integration.
        sf::Clock _imgui_clock;

        // Centralized UI manager that wraps ImGui-SFML usage.
        std::unique_ptr<UIManager> _ui;
    };
} // namespace Zia::engine

