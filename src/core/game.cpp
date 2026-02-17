// Connects the platformer Game implementation to the reusable engine infrastructure.

#include "mario/core/Game.hpp"
#include "mario/core/Menuscene.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

#include <memory>
#include <imgui.h>
#include <imgui-SFML.h>

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
        if (!ImGui::SFML::Init(_renderer.window())) {
            // Handle error if needed
        }
    }

    // Used by: Game::run(), main.cpp (explicit shutdown)
    // Shutdown the game, releasing or clearing owned resources and scenes.
    void Game::shutdown() {
        ImGui::SFML::Shutdown();
        // Clear active scenes to trigger their destructors and on_exit semantics.
        _scenes.clear();
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
        // Allow derived classes to set up an initial scene (push the menu/scene).
        before_loop();
        // Enter the fixed-timestep main loop which runs until _running is false.
        main_loop();
        // Perform cleanup after the loop ends.
        shutdown();
    }

    // Used by: menu_scene(push play_scene), Game::before_loop()
    // Push a new scene onto the stack and call lifecycle hooks.
    void Game::push_scene(std::shared_ptr<Scene> scene) {
        if (!scene) {
            return;
        }
        // If a scene is currently active, notify it that it loses focus.
        if (const auto current = current_scene()) {
            current->on_exit();
        }
        // Store the new scene (shared ownership) and notify it that it becomes active.
        _scenes.push_back(std::move(scene));
        _scenes.back()->on_enter();
    }

    // Used by: play_scene(to exit the current scene)
    // Pop the current scene and stop the game if no more scenes remain.
    void Game::pop_scene() {
        if (_scenes.empty()) {
            return;
        }
        // Notify the top scene it is exiting, then remove it.
        _scenes.back()->on_exit();
        _scenes.pop_back();
        // If no scenes remain, the game loop should terminate.
        if (_scenes.empty()) {
            _running = false;
        }
    }

    // Used by: Game::push_scene(), Game::before_loop(), Game::main_loop()
    // Return the currently active scene (or nullptr when none).
    std::shared_ptr<Scene> Game::current_scene() {
        if (_scenes.empty()) {
            return nullptr;
        }
        return _scenes.back();
    }

    // Used by: play_scene, menu_scene, rendering systems and HUD
    // Accessor for the renderer owned by Game.
    Renderer &Game::renderer() {
        return _renderer;
    }

    // Used by: play_scene::update(), menu_scene::update()
    // Accessor for the input manager owned by Game.
    InputManager &Game::input() {
        return _input;
    }

    // Used by: play_scene::on_enter() (loads textures), render systems
    // Accessor for the asset manager owned by Game.
    AssetManager &Game::assets() {
        return _assets;
    }

    // Used by: play_scene(update/render systems)
    // Accessor for the entity manager (ECS registry) owned by Game.
    EntityManager &Game::entity_manager() {
        return _entities;
    }

    // Used by: Game::run(); can be overridden by derived classes to push an initial scene
    // Hook called before entering the main loop: ensure an initial scene exists.
    void Game::before_loop() {
        // If no scene is present, push the menu scene as initial.
        if (!current_scene()) {
            push_scene(std::make_shared<MenuScene>(*this));
        }
    }

    // Used by: Game::run()
    // The main fixed-timestep loop: updates and renders the current scene.
    // Cache the current scene per-frame and use clock.restart() for clearer dt.
    void Game::main_loop() {
        sf::Clock clock;
        constexpr sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);

        while (_running) {
            const auto scene = current_scene();
            if (!scene) break;

            // ImGui process events
            sf::RenderWindow& window = _renderer.window();
            while (const auto event = window.pollEvent()) {
                ImGui::SFML::ProcessEvent(window, *event);
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            // Compute delta time in seconds since the last frame.
            const float dt = clock.restart().asSeconds();

            // ImGui Update
            ImGui::SFML::Update(window, _imgui_clock.restart());

            // Simple demo window
            ImGui::Begin("ImGui working!");
            ImGui::Text("Hello, SFML 3 + ImGui!");
            ImGui::End();

            scene->update(dt);

            _renderer.begin_frame();
            scene->render();
            ImGui::SFML::Render(window);
            _renderer.end_frame();

            const sf::Time elapsed = clock.getElapsedTime();
            if (elapsed < target_frame_time) {
                sf::sleep(target_frame_time - elapsed);
            }
        }
    }
} // namespace mario
