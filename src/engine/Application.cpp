// Minimal implementation of the engine-level Application. It reuses the existing Mario managers
// and adapts the main loop from the previous Game implementation. The goal is to remain
// source-compatible with the rest of the project while introducing an engine abstraction.

#include "mario/engine/Application.hpp"
#include "mario/engine/adapters/SceneAdapter.hpp"

#include "mario/engine/adapters/RendererAdapter.hpp"
#include "mario/engine/adapters/InputAdapter.hpp"
#include "mario/engine/adapters/AssetManagerAdapter.hpp"
#include "mario/engine/adapters/EntityManagerAdapter.hpp"

#include <iostream>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

namespace mario::engine {
    Application::Application(std::string_view title) {
        // Create owned concrete subsystems.
        auto renderer = std::make_shared<mario::Renderer>();
        // If a title was provided, update the renderer window title.
        if (!title.empty()) {
            renderer->window().setTitle(sf::String(std::string(title)));
        }
        auto input = std::make_shared<mario::InputManager>();
        auto assets = std::make_shared<mario::AssetManager>();
        auto entities = std::make_shared<mario::EntityManager>();

        // Initialize interface adapters.
        _renderer_iface = std::make_shared<engine::adapters::RendererAdapter>(renderer);
        _input_iface = std::make_shared<engine::adapters::InputAdapter>(input);
        _assets_iface = std::make_shared<engine::adapters::AssetManagerAdapter>(assets);
        _entities_iface = std::make_shared<engine::adapters::EntityManagerAdapter>(entities);

        // Initialize UI manager with default implementation.
        _ui = std::make_unique<UIManager>();
    }

    Application::Application(std::shared_ptr<IRenderer> renderer,
                             std::shared_ptr<IInput> input,
                             std::shared_ptr<IAssetManager> assets,
                             std::shared_ptr<IEntityManager> entities)
        : _renderer_iface(std::move(renderer)), _input_iface(std::move(input)),
          _assets_iface(std::move(assets)), _entities_iface(std::move(entities)),
          _ui(std::make_unique<UIManager>()) {

        // Ensure we have a valid interface for each subsystem.
        if (!_renderer_iface) {
            _renderer_iface = std::make_shared<engine::adapters::RendererAdapter>(std::make_shared<mario::Renderer>());
        }
        if (!_input_iface) {
            _input_iface = std::make_shared<engine::adapters::InputAdapter>(std::make_shared<mario::InputManager>());
        }
        if (!_assets_iface) {
            _assets_iface = std::make_shared<engine::adapters::AssetManagerAdapter>(std::make_shared<mario::AssetManager>());
        }
        if (!_entities_iface) {
            _entities_iface = std::make_shared<engine::adapters::EntityManagerAdapter>(std::make_shared<mario::EntityManager>());
        }
    }

    Application::~Application() = default;

    void Application::initialize() {
        _running = true;
        // Initialize UI manager with the renderer's window. If it fails, log a warning but continue.
        if (_ui && !_ui->init(_renderer_iface->window())) {
            std::cerr << "Warning: UI Manager failed to initialize. ImGui features will be unavailable." << std::endl;
        }
    }

    void Application::shutdown() {
        if (_ui) _ui->shutdown();
        _scenes.clear();
        _assets_iface->unload_all();
        _entities_iface->clear();
        _running = false;
    }

    void Application::run() {
        initialize();
        before_loop();
        main_loop();
        shutdown();
    }

    void Application::push_scene(std::shared_ptr<IScene> scene) {
        if (!scene) return;
        if (const auto current = current_scene()) {
            current->on_exit();
        }
        _scenes.push_back(std::move(scene));
        _scenes.back()->on_enter();
    }

    void Application::pop_scene() {
        if (_scenes.empty()) return;
        _scenes.back()->on_exit();
        _scenes.pop_back();
        if (_scenes.empty()) {
            _running = false;
        }
    }

    std::shared_ptr<IScene> Application::current_scene() {
        if (_scenes.empty()) return nullptr;
        return _scenes.back();
    }

    IRenderer &Application::renderer() { return *_renderer_iface; }

    IInput &Application::input() { return *_input_iface; }

    IAssetManager &Application::assets() { return *_assets_iface; }

    IEntityManager &Application::entity_manager() { return *_entities_iface; }

    mario::EntityManager &Application::underlying_entity_manager() { return _entities_iface->underlying(); }

    UIManager &Application::ui() { return *_ui; }

    void Application::before_loop() {
        // Default: engine does not assume any game-specific initial scene. Games should override
        // or the game-level wrapper (`mario::Game`) should push an initial scene as needed.
    }

    void Application::main_loop() {
        sf::Clock clock;
        constexpr sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);

        while (_running) {
            const auto scene = current_scene();
            if (!scene) break;

            // ImGui process events
            sf::RenderWindow &window = _renderer_iface->window();
            while (const auto event = window.pollEvent()) {
                if (_ui) _ui->process_event(window, *event);
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            const float dt = clock.restart().asSeconds();

            scene->update(dt);

            if (_ui) {
                _ui->update(window, _imgui_clock);
                _ui->build();
            }

            _renderer_iface->begin_frame();
            scene->render();
            if (_ui) _ui->render(window);
            _renderer_iface->end_frame();

            const sf::Time elapsed = clock.getElapsedTime();
            if (elapsed < target_frame_time) {
                sf::sleep(target_frame_time - elapsed);
            }
        }
    }
} // namespace mario::engine
