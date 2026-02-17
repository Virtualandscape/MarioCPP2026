// Implements the PlayScene class, which manages the main gameplay scene, including level loading, player spawning, and HUD updates.
// Handles entering and exiting the play scene, updating game logic, and rendering the game world and HUD.

#include "mario/core/PlayScene.hpp"
#include "mario/core/Game.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"
#include "mario/helpers/Spawner.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/helpers/Constants.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/systems/CollisionSystem.hpp"
#include "mario/systems/InspectorSystem.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <future>
#include <thread>
#include <filesystem>
#include <optional>
#include <SFML/Graphics/Image.hpp>

namespace mario {

    // Used by: Game state manager / state stack
    // Constructor initializes the PlayScene with a reference to the game and optional level path.
    // Stores a reference to the Game instance and prepares the HUD with the renderer.
    PlayScene::PlayScene(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    // Used by: Game state manager / state stack
    // Alternate constructor that pre-selects a level to load when entering the scene.
    PlayScene::PlayScene(Game &game, std::string level_path) : _game(game), _player_id(0),
                                                               _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {}

    // Used by: Game::push_scene / scene manager when entering this scene
    // Called when entering the play scene. Loads level assets, spawns entities and builds system pipelines.
    void PlayScene::on_enter() {
        // Load the level data from the configured path into the Level object.
        _level.load(_current_level_path);

        auto& registry = _game.entity_manager();

        // Background path is used for preloading; fetch it before starting preload.
        const std::string &level_bg_path = _level.background_path();

        // Local resolver used by background thread to find asset file paths.
        // This helper tries multiple relative locations to find the file on disk.
        auto resolve_asset_path_local = [](std::string_view path) -> std::optional<std::filesystem::path> {
            std::filesystem::path base(path);
            const std::filesystem::path cwd = std::filesystem::current_path();
            const std::filesystem::path tries[] = {
                base,
                cwd / base,
                cwd / ".." / base,
                cwd / ".." / ".." / base,
                cwd / ".." / ".." / ".." / base,
            };
            for (const auto &candidate : tries) {
                if (std::filesystem::exists(candidate)) return candidate;
            }
            return std::nullopt;
        };

        // Preload common textures (player, clouds, background layers) to avoid IO during spawn.
        {
            // Logging removed per request; preserve no-op logger for possible future use.
            auto log_line = [&](const std::string &/*s*/){ };

            // Light assets to load synchronously (fast, small files)
            std::vector<std::pair<int,std::string>> light_list;
            light_list.emplace_back(mario::constants::PLAYER_IDLE_ID, "assets/Sprites/Player64/Idle.png");
            light_list.emplace_back(mario::constants::PLAYER_RUN_ID, "assets/Sprites/Player64/Run.png");
            light_list.emplace_back(mario::constants::PLAYER_JUMP_ID, "assets/Sprites/Player64/Jump.png");
            light_list.emplace_back(mario::constants::CLOUD_MEDIUM_ID, "assets/environment/background/cloud_medium.png");
            light_list.emplace_back(mario::constants::CLOUD_SMALL_ID, "assets/environment/background/cloud_small.png");
            light_list.emplace_back(mario::constants::BACKGROUND_TEXTURE_ID, "assets/environment/background/sky.png");

            // Heavy assets to load asynchronously (large, slower to decode)
            std::vector<std::pair<int,std::string>> heavy_list;
            heavy_list.emplace_back(mario::constants::CLOUD_BIG_ID, "assets/environment/background/cloud_big.png");
            // If the level defines specific layers, mark them heavy (mountains example)
            if (!level_bg_path.empty()) {
                heavy_list.emplace_back(mario::constants::BACKGROUND_TEXTURE_ID + 1, std::string(_level.background_path()));
                for (const auto &layer: _level.background_layers()) {
                    heavy_list.emplace_back(mario::constants::BACKGROUND_TEXTURE_ID + 1, layer.path);
                }
            } else {
                // Default mountains layer used by levels
                heavy_list.emplace_back(mario::constants::BACKGROUND_TEXTURE_ID + 1, "assets/environment/background/mountains.png");
            }

            // Load light assets synchronously.
            for (const auto &p : light_list) {
                // Load small/fast textures synchronously to ensure they're available.
                (void)_game.assets().load_texture(p.first, p.second);
            }

            // Launch an async task to load heavy assets without blocking; update() will finalize textures progressively.
            // The async worker decodes images into sf::Image and pushes them to the AssetManager for main-thread finalization.
            _assets_loading = true;
            _asset_loading_future = std::async(std::launch::async, [this, heavy_list, resolve_asset_path_local]() {
                for (const auto &p : heavy_list) {
                    // Local decoded image buffer used to hold decoded pixels off the main thread.
                    sf::Image img;
                    bool ok = false;
                    const auto resolved = resolve_asset_path_local(p.second);
                    if (resolved) {
                        ok = img.loadFromFile(resolved->string());
                        if (ok) {
                            // Push the decoded image to the global asset manager. The main thread will finalize to a texture.
                            _game.assets().push_decoded_image(p.first, std::move(img));
                        }
                    }
                }
                // mark done by setting assets_loading false on the main thread later after future ready
            });

            // Note: do not block here — update() will call finalize_decoded_images each frame.
        }

        // Background loading (level dependent)
        // If the level defines a background image path, load it and create background entities.
        if (!level_bg_path.empty()) {
            if (_game.assets().load_texture(mario::constants::BACKGROUND_TEXTURE_ID, level_bg_path)) {
                // Create the main background entity. BackgroundSystem will attach a BackgroundComponent
                // configured with scale, parallax and tiling parameters.
                _background_system.create_background_entity(registry, mario::constants::BACKGROUND_TEXTURE_ID, true, BackgroundComponent::ScaleMode::Fill,
                                         _level.background_scale(), 0.0f, false, false, 0.0f, 0.0f);
            }

            // Load additional background layers defined in the level file.
            int texture_id = mario::constants::BACKGROUND_TEXTURE_ID + 1;
            for (const auto &layer: _level.background_layers()) {
                if (_game.assets().load_texture(texture_id, layer.path)) {
                    // Create a background entity for this layer; parallax and repeating handled by BackgroundSystem.
                    _background_system.create_background_entity(registry, texture_id, true, BackgroundComponent::ScaleMode::Fit, layer.scale,
                                             layer.parallax, layer.repeat, layer.repeat_x, 0.0f, 0.0f);
                }
                ++texture_id;
            }
        }

        // Initialize clouds if the level enables them. CloudSystem will create cloud entities/components.
        if (_level.clouds_enabled()) {
            _cloud_system.initialize(_game.assets(), registry);
        }

        // Spawn entities declared in the level (player and enemies).
        bool player_spawned = false;
        if (const auto tile_map = _level.tile_map()) {
            TileMap &tm = *tile_map;
            const auto tile_size = static_cast<float>(tm.tile_size());
            if (tile_size > 0.0f) {
                for (const auto &spawn: _level.entity_spawns()) {
                    // Decide spawn type and delegate to Spawner helper which sets up components.
                    if (spawn.type == "player" || spawn.type == "Player") {
                        // Spawn the player using the Spawner helper which configures components and assets.
                        _player_id = Spawner::spawn_player(registry, spawn, _game.assets());
                        player_spawned = true;
                    } else {
                        // Spawn a generic enemy entity according to the spawn entry.
                        Spawner::spawn_enemy(registry, spawn);
                    }
                }
            }
        }
        if (!player_spawned) {
            // Fallback: spawn a default player if no player spawn was found in the level.
            _player_id = Spawner::spawn_player_default(registry, _game.assets());
        }

        // Initialize camera via CameraSystem. This sets the viewport and optionally centers on the player.
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            // Apply an initial horizontal offset to make enter-damping visible to the player.
            _camera_system.initialize(registry, *camera, viewport.x, viewport.y, _player_id, -100.0f, 0.0f);
        }

        // Initialize inspector system (font via AssetManager)
        _inspector_system.initialize(_game.assets());

        // Mark scene as running and prepare the per-frame system pipelines.
        _running = true;
        _level_transition_delay = 0.5f; // LevelTransitionCooldown
        setup_systems();
    }

    // Used by: Game::pop_scene / scene manager when exiting this scene
    // Called when exiting the play scene. Clears ECS registry and unloads level resources.
    void PlayScene::on_exit() {
        // Remove all entities/components related to this level.
        auto& registry = _game.entity_manager();
        registry.clear();
        _player_id = 0;
        _level.unload();
    }

    // Used by: Game main loop (per-frame update)
    // Updates the game logic, including input handling, ECS systems and level logic.
    void PlayScene::update(float dt) {
        // Poll input and handle user-driven actions (escape, debug toggle, etc.).
        handle_input();
        auto& registry = _game.entity_manager();

        // Finalize any decoded images from background thread into textures on main thread.
        _game.assets().finalize_decoded_images();
        // If the future is valid and ready, reset the loading flag.
        if (_assets_loading) {
            if (_asset_loading_future.valid()) {
                using namespace std::chrono_literals;
                if (_asset_loading_future.wait_for(0ms) == std::future_status::ready) {
                    _assets_loading = false;
                }
            } else {
                _assets_loading = false;
            }
        }

        // Execute the ordered update pipeline built in setup_systems.
        run_update_systems(registry, dt);

        // Update camera systems after core simulation so camera follows the player smoothly.
        if (auto camera_ptr = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            _camera_system.update(registry, *camera_ptr, dt, viewport.x, viewport.y, _player_id);
        }

        // Let level perform any temporal updates (animations, timers, transitions).
        _level.update(dt);

        // Handle any pending level transitions requested by systems.
        handle_level_transitions();
    }


    // Used by: LevelSystem and internal scene transition logic
    // Check and apply pending level transitions (reload/unload/load next level).
    void PlayScene::handle_level_transitions() {
        if (_level_transition_pending) {
            _level_transition_pending = false;
            on_exit();
            on_enter();
        }
    }

    // Used by: Game main loop to process input events
    // Process input events and global actions like exiting the scene or toggling debug drawing.
    void PlayScene::handle_input() {
        _game.input().poll();
        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _game.pop_scene();
        }

        // Toggle debug bounding boxes and inspector overlay on rising edge of the ToggleDebug action.
        bool current = _game.input().is_pressed(InputManager::Action::ToggleDebug);
        bool rising = (current && !_debug_toggle_last_state);
        if (rising) {
            _game.renderer().toggle_debug_bboxes();
            _inspector_system.toggle_enabled();
        }
        _debug_toggle_last_state = current;
    }

    // Used by: on_enter to build per-frame pipelines
    // Build the ordered per-frame system pipelines as lambda callbacks.
    void PlayScene::setup_systems() {
        // Clear any previous pipeline entries.
        _update_systems.clear();
        // Player input and movement controller must run early so later systems see an updated control state.
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            _player_controller.update(registry, _game.input(), dt);
        });
        // Update animations after player and AI logic so they reflect the current state.
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            _animation_system.update(registry, dt);
        });
        // Run enemy AI and movement which may depend on the current tilemap.
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                _enemy_system.update(registry, *tile_map, dt);
            }
        });
        // Physics simulation (collisions, velocity integration) runs after motion inputs.
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            _physics.update(registry, dt);
        });
        // Cloud system updates visual cloud entities (non-critical gameplay elements).
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            _cloud_system.update(registry, dt);
        });
        // Tile/level collision detection and resolution.
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                CollisionSystem::update(registry, *tile_map, dt);
            }
        });
        // Level transitions check should run after all simulation so it can act on final state.
        _update_systems.emplace_back([this](EntityManager& registry, float dt) {
            if (LevelSystem::handle_transitions(registry, _player_id, _level, _current_level_path, _level_transition_delay, dt)) {
                _level_transition_pending = true;
            }
        });

        // Build render callbacks: these are executed each frame with the current camera context.
        _render_systems.clear();
        _render_systems.emplace_back([this](EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera){
            // Render background layers sorted by parallax to create depth.
            static thread_local std::vector<EntityID> bg_entities;
            registry.get_entities_with<BackgroundComponent>(bg_entities);
            std::sort(bg_entities.begin(), bg_entities.end(), [&](EntityID a, EntityID b) {
                auto a_opt = registry.get_component<BackgroundComponent>(a);
                auto b_opt = registry.get_component<BackgroundComponent>(b);
                if (!a_opt || !b_opt) return false;
                return (a_opt->get().parallax < b_opt->get().parallax);
            });
            for (auto entity: bg_entities) {
                if (auto bg_opt = registry.get_component<BackgroundComponent>(entity)) {
                    _background_system.render(renderer, camera, assets, bg_opt->get());
                }
            }
            // Render clouds, level geometry, sprites and debug overlays.
            _cloud_system.render(renderer, camera, assets, registry);
            _level.render(renderer);
            _sprite_render_system.render(renderer, camera, registry, assets);
            _debug_draw_system.render(renderer, camera, registry);

            // Update and draw HUD elements (level name, score, etc.).
            std::string level_name = "Level 1";
            if (_current_level_path == mario::constants::LEVEL2_PATH) {
                level_name = "Level 2";
            }
            _hud.set_level_name(level_name);
            _hud.render();
        });

        // Inspector overlay should render after everything else so it's on top
        _render_systems.emplace_back([this](EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera){
            _inspector_system.render(renderer, camera, registry, assets);
        });
    }

    // Used by: Game main loop to draw a frame
    // Renders the game world and HUD. Note: begin_frame()/end_frame() are handled by Game::main_loop().
    void PlayScene::render() {
        // Render logic only — the Game loop is responsible for begin_frame()/end_frame().

        // Compute a camera pointer: if the level supplies a camera, use it; otherwise use a dummy camera.
        auto camera_ptr = _level.camera();
        Camera dummy;
        // Create a local camera view (copy) to pass into render systems. This avoids pointer dereferencing warnings.
        Camera camera_view = camera_ptr ? *camera_ptr : dummy;
        // Apply the current camera position to the renderer before rendering (dummy view is safe).
        // Copy camera coordinates to local variables to avoid analyzer warnings on complex expressions.
        const float cam_x = camera_view.x();
        const float cam_y = camera_view.y();
        _game.renderer().set_camera(cam_x, cam_y);

        // Execute the render pipeline with camera context.
        run_render_systems(_game.entity_manager(), camera_view);
    }

    // Used by: update (executes update pipeline)
    // Execute the stored update callbacks in order.
    void PlayScene::run_update_systems(EntityManager &registry, float dt) {
        for (auto &sys : _update_systems) {
            sys(registry, dt);
        }
    }

    // Used by: render (executes render pipeline)
    // Execute the stored render callbacks in order. Each callback receives the renderer and assets.
    void PlayScene::run_render_systems(EntityManager &registry, const Camera &camera) {
        for (auto &sys : _render_systems) {
            sys(registry, _game.renderer(), _game.assets(), camera);
        }
    }

    // Used by: Game main loop to check whether this scene remains active
    // Query whether the scene should keep running. This checks both the internal running flag
    // and whether the renderer window is still open.
    bool PlayScene::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario

