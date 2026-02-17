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

Application::Application(std::string_view title)
    : _renderer_adapter(nullptr), _input_adapter(nullptr), _assets_adapter(nullptr), _entities_adapter(nullptr) {
    // Create owned subsystems. Renderer constructed with default constructor.
    _renderer = std::make_unique<mario::Renderer>();
    // If a title was provided, update the renderer window title.
    if (!title.empty()) {
        _renderer->window().setTitle(sf::String(std::string(title)));
    }
    _input = std::make_unique<mario::InputManager>();
    _assets = std::make_unique<mario::AssetManager>();
    _entities = std::make_unique<mario::EntityManager>();
}

Application::Application(std::shared_ptr<IRenderer> renderer,
                         std::shared_ptr<IInput> input,
                         std::shared_ptr<IAssetManager> assets,
                         std::shared_ptr<IEntityManager> entities)
    : _renderer_adapter(std::move(renderer)), _input_adapter(std::move(input)),
      _assets_adapter(std::move(assets)), _entities_adapter(std::move(entities)) {
    // If adapters expose underlying concrete objects via their adapter classes, prefer them.
    // Otherwise, fall back to creating concrete subsystem instances.
    if (_renderer_adapter) {
        // Try to get concrete renderer from known Adapter type
        if (auto ra = std::dynamic_pointer_cast<engine::adapters::RendererAdapter>(_renderer_adapter)) {
            _renderer = ra->underlying();
        }
    }
    if (!_renderer) _renderer = std::make_shared<mario::Renderer>();

    if (_input_adapter) {
        if (auto ia = std::dynamic_pointer_cast<engine::adapters::InputAdapter>(_input_adapter)) {
            _input = ia->underlying();
        }
    }
    if (!_input) _input = std::make_shared<mario::InputManager>();

    if (_assets_adapter) {
        if (auto aa = std::dynamic_pointer_cast<engine::adapters::AssetManagerAdapter>(_assets_adapter)) {
            _assets = aa->underlying();
        }
    }
    if (!_assets) _assets = std::make_shared<mario::AssetManager>();

    if (_entities_adapter) {
        if (auto ea = std::dynamic_pointer_cast<engine::adapters::EntityManagerAdapter>(_entities_adapter)) {
            _entities = ea->underlying();
        }
    }
    if (!_entities) _entities = std::make_shared<mario::EntityManager>();
}

Application::~Application() = default;

void Application::initialize() {
    _running = true;
    // Initialize UI manager with the renderer's window. If it fails, log a warning but continue.
    if (!_ui.init(_renderer->window())) {
        std::cerr << "Warning: UI Manager failed to initialize. ImGui features will be unavailable." << std::endl;
    }
}

void Application::shutdown() {
    _ui.shutdown();
    _scenes.clear();
    _assets->unload_all();
    _entities->clear();
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

mario::Renderer &Application::renderer() { return *_renderer; }

mario::InputManager &Application::input() { return *_input; }

mario::AssetManager &Application::assets() { return *_assets; }

mario::EntityManager &Application::entity_manager() { return *_entities; }

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
        sf::RenderWindow &window = _renderer->window();
        while (const auto event = window.pollEvent()) {
            _ui.process_event(window, *event);
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        const float dt = clock.restart().asSeconds();

        scene->update(dt);

        _ui.update(window, _imgui_clock);
        _ui.build();

        _renderer->begin_frame();
        scene->render();
        _ui.render(window);
        _renderer->end_frame();

        const sf::Time elapsed = clock.getElapsedTime();
        if (elapsed < target_frame_time) {
            sf::sleep(target_frame_time - elapsed);
        }
    }
}

} // namespace mario::engine

