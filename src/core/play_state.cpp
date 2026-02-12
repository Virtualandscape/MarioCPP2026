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
    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0),
                                                               _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {}

    // Called when entering the play state. Loads the level, background, and spawns the player.
    void PlayState::on_enter() {
        // Load level
        _level.load(_current_level_path);
        auto& registry = _game.entity_manager();

        // Background loading (level dependent)
        const std::string &level_bg_path = _level.background_path();
        if (!level_bg_path.empty()) {
            if (_game.assets().load_texture(mario::constants::BACKGROUND_TEXTURE_ID, level_bg_path)) {
                // Create the main background entity. BackgroundSystem will create an entity and attach a BackgroundComponent
                // configured with scale, parallax and tiling parameters.
                _background_system.create_background_entity(registry, mario::constants::BACKGROUND_TEXTURE_ID, true, BackgroundComponent::ScaleMode::Fill,
                                         _level.background_scale(), 0.0f, false, false, 0.0f, 0.0f);
            }

            // Load additional background layers from level data
            int texture_id = mario::constants::BACKGROUND_TEXTURE_ID + 1;
            for (const auto &layer: _level.background_layers()) {
                if (_game.assets().load_texture(texture_id, layer.path)) {
                    // Create a background entity for this layer (parallax, repeat and offsets are handled by BackgroundSystem).
                    _background_system.create_background_entity(registry, texture_id, true, BackgroundComponent::ScaleMode::Fit, layer.scale,
                                             layer.parallax, layer.repeat, layer.repeat_x, 0.0f, 0.0f);
                }
                ++texture_id;
            }
        }

        // Initialize clouds if enabled for this level
        if (_level.clouds_enabled()) {
            // Initialize cloud entities via CloudSystem: it creates and configures cloud entities/components using the AssetManager and registry.
            _cloud_system.initialize(_game.assets(), registry);
        }

        // Spawn entities
        bool player_spawned = false;
        if (const auto tile_map = _level.tile_map()) {
            TileMap &tm = *tile_map;
            const auto tile_size = static_cast<float>(tm.tile_size());
            if (tile_size > 0.0f) {
                for (const auto &spawn: _level.entity_spawns()) {
                    if (spawn.type == "player" || spawn.type == "Player") {
                        _player_id = Spawner::spawn_player(registry, spawn, _game.assets());
                        player_spawned = true;
                    } else {
                        Spawner::spawn_enemy(registry, spawn);
                    }
                }
            }
        }
        if (!player_spawned) {
            _player_id = Spawner::spawn_player_default(registry, _game.assets());
        }

        // Initialize camera via CameraSystem (sets viewport, centers on player if available and applies initial offset)
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            // Apply initial horizontal offset (-100) to make the damping animation visible on enter
            _camera_system.initialize(registry, *camera, viewport.x, viewport.y, _player_id, -100.0f, 0.0f);
        }

        _running = true;
        _level_transition_delay = 0.5f; // LevelTransitionCooldown
        auto& scheduler = _game.system_scheduler();
        scheduler.clear();
        setup_systems();
    }

    // Called when exiting the play state. Unloads the level and clears the registry.
    void PlayState::on_exit() {
        // Clear registry to remove entities/components from the previous level
        auto& registry = _game.entity_manager();
        registry.clear();
        _player_id = 0;
        _level.unload();
    }

    // Updates the game logic, including the level, player, and HUD.
    void PlayState::update(float dt) {
        handle_input();
        auto& registry = _game.entity_manager();
        _game.system_scheduler().update(registry, dt);

        if (auto camera_ptr = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            _camera_system.update(registry, *camera_ptr, dt, viewport.x, viewport.y, _player_id);
        }

        _level.update(dt);

        handle_level_transitions();
    }


    void PlayState::handle_level_transitions() {
        if (_level_transition_pending) {
            _level_transition_pending = false;
            on_exit();
            on_enter();
        }
    }

    void PlayState::handle_input() {
        _game.input().poll();
        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _game.pop_state();
        }

        // Toggle debug bounding boxes on rising edge of H key
        bool current = _game.input().is_pressed(InputManager::Action::ToggleDebug);
        if (current && !_debug_toggle_last_state) {
            _game.renderer().toggle_debug_bboxes();
        }
        _debug_toggle_last_state = current;
    }

    void PlayState::setup_systems() {
        auto& scheduler = _game.system_scheduler();
        scheduler.add_system([this](EntityManager& registry, float dt) {
            _player_controller.update(registry, _game.input(), dt);
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            _animation_system.update(registry, dt);
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                _enemy_system.update(registry, *tile_map, dt);
            }
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            _physics.update(registry, dt);
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            _cloud_system.update(registry, dt);
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                CollisionSystem::update(registry, *tile_map, dt);
            }
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            if (LevelSystem::handle_transitions(registry, _player_id, _level, _current_level_path, _level_transition_delay, dt)) {
                _level_transition_pending = true;
            }
        });

        scheduler.add_render_system([this](EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera) {
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
            _cloud_system.render(renderer, camera, assets, registry);
            _level.render(renderer);
            _sprite_render_system.render(renderer, camera, registry, assets);
            _debug_draw_system.render(renderer, camera, registry);

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
        _game.renderer().begin_frame();

        auto camera_ptr = _level.camera();
        Camera dummy;
        Camera *cam = camera_ptr ? camera_ptr.get() : &dummy;
        if (camera_ptr) {
            _game.renderer().set_camera(cam->x(), cam->y());
        }

        _game.system_scheduler().render(_game.entity_manager(), _game.renderer(), _game.assets(), *cam);

        _game.renderer().end_frame();
    }

    bool PlayState::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario
