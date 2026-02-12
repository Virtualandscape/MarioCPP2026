// Implements the PlayState class, which manages the main gameplay state, including level loading, player spawning, and HUD updates.
// Handles entering and exiting the play state, updating game logic, and rendering the game world and HUD.

#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"
#include "mario/helpers/Spawner.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/helpers/Constants.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/systems/CollisionSystem.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace mario {

    // Constructor initializes the PlayState with a reference to the game and optional level path.
    // Stores a reference to the Game instance and prepares the HUD with the renderer.
    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    // Alternate constructor that pre-selects a level to load when entering the state.
    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0),
                                                               _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {}

    // Called when entering the play state. Loads level assets, spawns entities and builds system pipelines.
    void PlayState::on_enter() {
        // Load the level data from the configured path into the Level object.
        _level.load(_current_level_path);
        auto& registry = _game.entity_manager();

        // Background loading (level dependent)
        // If the level defines a background image path, load it and create background entities.
        const std::string &level_bg_path = _level.background_path();
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

        // Mark state as running and prepare the per-frame system pipelines.
        _running = true;
        _level_transition_delay = 0.5f; // LevelTransitionCooldown
        setup_systems();
    }

    // Called when exiting the play state. Clears ECS registry and unloads level resources.
    void PlayState::on_exit() {
        // Remove all entities/components related to this level.
        auto& registry = _game.entity_manager();
        registry.clear();
        _player_id = 0;
        _level.unload();
    }

    // Updates the game logic, including input handling, ECS systems and level logic.
    void PlayState::update(float dt) {
        // Poll input and handle user-driven actions (escape, debug toggle, etc.).
        handle_input();
        auto& registry = _game.entity_manager();
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


    // Check and apply pending level transitions (reload/unload/load next level).
    void PlayState::handle_level_transitions() {
        if (_level_transition_pending) {
            _level_transition_pending = false;
            on_exit();
            on_enter();
        }
    }

    // Process input events and global actions like exiting the state or toggling debug drawing.
    void PlayState::handle_input() {
        _game.input().poll();
        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _game.pop_state();
        }

        // Toggle debug bounding boxes on rising edge of the ToggleDebug action.
        bool current = _game.input().is_pressed(InputManager::Action::ToggleDebug);
        if (current && !_debug_toggle_last_state) {
            _game.renderer().toggle_debug_bboxes();
        }
        _debug_toggle_last_state = current;
    }

    // Build the ordered per-frame system pipelines as lambda callbacks.
    void PlayState::setup_systems() {
        // Clear any previous pipeline entries.
        _update_systems.clear();
        // Player input and movement controller must run early so subsequent systems see updated control state.
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            _player_controller.update(registry, _game.input(), dt);
        });
        // Update animations after player and AI logic so they reflect the current state.
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            _animation_system.update(registry, dt);
        });
        // Run enemy AI and movement which may depend on the current tilemap.
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                _enemy_system.update(registry, *tile_map, dt);
            }
        });
        // Physics simulation (collisions, velocity integration) runs after motion inputs.
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            _physics.update(registry, dt);
        });
        // Cloud system updates visual cloud entities (non-critical gameplay elements).
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            _cloud_system.update(registry, dt);
        });
        // Tile/level collision detection and resolution.
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                CollisionSystem::update(registry, *tile_map, dt);
            }
        });
        // Level transitions check should run after all simulation so it can act on final state.
        _update_systems.push_back([this](EntityManager& registry, float dt) {
            if (LevelSystem::handle_transitions(registry, _player_id, _level, _current_level_path, _level_transition_delay, dt)) {
                _level_transition_pending = true;
            }
        });

        // Build render callbacks: these are executed each frame with the current camera context.
        _render_systems.clear();
        _render_systems.push_back([this](EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera){
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
    }

    // Renders the game world and HUD.
    void PlayState::render() {
        // Prepare the renderer for a new frame (clears screen and polls OS events).
        _game.renderer().begin_frame();

        // Compute camera pointer: if the level supplies a camera, use it; otherwise use a dummy camera.
        auto camera_ptr = _level.camera();
        Camera dummy;
        Camera *cam = camera_ptr ? camera_ptr.get() : &dummy;
        if (camera_ptr) {
            _game.renderer().set_camera(cam->x(), cam->y());
        }

        // Execute render pipeline with camera context.
        run_render_systems(_game.entity_manager(), *cam);

        // Present the rendered frame to the display.
        _game.renderer().end_frame();
    }

    // Execute the stored update callbacks in order.
    void PlayState::run_update_systems(EntityManager &registry, float dt) {
        for (auto &sys : _update_systems) {
            sys(registry, dt);
        }
    }

    // Execute the stored render callbacks in order. Each callback receives the renderer and assets.
    void PlayState::run_render_systems(EntityManager &registry, const Camera &camera) {
        for (auto &sys : _render_systems) {
            sys(registry, _game.renderer(), _game.assets(), camera);
        }
    }

    // Query whether the state should keep running. This checks both the internal running flag
    // and whether the renderer window is still open.
    bool PlayState::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario
