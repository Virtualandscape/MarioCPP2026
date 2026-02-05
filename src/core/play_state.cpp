// Implements the PlayState class, which manages the main gameplay state, including level loading, player spawning, and HUD updates.
// Handles entering and exiting the play state, updating game logic, and rendering the game world and HUD.

#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"
#include "mario/util/Spawner.hpp"
#include "mario/world/TileMap.hpp"

#include <algorithm>
#include <string>

namespace mario {
    namespace {
        constexpr int BACKGROUND_TEXTURE_ID = 1000;
    }

    // Constructor initializes the PlayState with a reference to the game and optional level path.
    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0),
                                                               _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {
    }

    // Called when entering the play state. Loads the level, background, and spawns the player.
    void PlayState::on_enter() {
        // Load level
        _level.load(_current_level_path);

        // Background loading (level dependent)
        const std::string &level_bg_path = _level.background_path();
        if (!level_bg_path.empty()) {
            if (_game.assets().load_texture(BACKGROUND_TEXTURE_ID, level_bg_path)) {
                // Create the main background entity. BackgroundSystem will create an entity and attach a BackgroundComponent
                // configured with scale, parallax and tiling parameters.
                _background_system.create_background_entity(_registry, BACKGROUND_TEXTURE_ID, true, BackgroundComponent::ScaleMode::Fill,
                                         _level.background_scale(), 0.0f, false, false, 0.0f, 0.0f);
            }

            // Load additional background layers from level data
            int texture_id = BACKGROUND_TEXTURE_ID + 1;
            for (const auto &layer: _level.background_layers()) {
                if (_game.assets().load_texture(texture_id, layer.path)) {
                    // Create a background entity for this layer (parallax, repeat and offsets are handled by BackgroundSystem).
                    _background_system.create_background_entity(_registry, texture_id, true, BackgroundComponent::ScaleMode::Fit, layer.scale,
                                             layer.parallax, layer.repeat, layer.repeat_x, 0.0f, 0.0f);
                }
                ++texture_id;
            }
        }

        // Initialize clouds if enabled for this level
        if (_level.clouds_enabled()) {
            // Initialize cloud entities via CloudSystem: it creates and configures cloud entities/components using the AssetManager and registry.
            _cloud_system.initialize(_game.assets(), _registry);
        }

        // Spawn entities
        bool player_spawned = false;
        if (const auto tile_map = _level.tile_map()) {
            TileMap &tm = *tile_map;
            const auto tile_size = static_cast<float>(tm.tile_size());
            if (tile_size > 0.0f) {
                for (const auto &spawn: _level.entity_spawns()) {
                    if (spawn.type == "player" || spawn.type == "Player") {
                        _player_id = Spawner::spawn_player(_registry, spawn, tile_size);
                        player_spawned = true;
                    } else {
                        Spawner::spawn_enemy(_registry, spawn, tile_size);
                    }
                }
            }
        }
        if (!player_spawned) {
            _player_id = Spawner::spawn_player_default(_registry);
        }

        // Initialize camera via CameraSystem (sets viewport, centers on player if available and applies initial offset)
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            // Apply initial horizontal offset (-100) to make the damping animation visible on enter
            _camera_system.initialize(_registry, *camera, viewport.x, viewport.y, _player_id, -100.0f, 0.0f);
        }

        _running = true;
        _level_transition_delay = 0.5f; // LevelTransitionCooldown
    }

    // Called when exiting the play state. Unloads the level and clears the registry.
    void PlayState::on_exit() {
        // Clear registry to remove entities/components from the previous level
        _registry.clear();
        _player_id = 0;
        _level.unload();
    }

    // Updates the game logic, including the level, player, and HUD.
    void PlayState::update(float dt) {
        handle_input();

        auto tile_map = _level.tile_map();

        // Update ECS systems
        _player_input.update(_registry, _game.input());
        _player_movement.update(_registry, dt);
        if (tile_map) {
            _enemy_system.update(_registry, *tile_map, dt);
        }
        _physics.update(_registry, dt);
        _cloud_system.update(_registry, dt);
        if (tile_map) {
            CollisionSystem::update(_registry, *tile_map, dt);
        }

        // Delegate camera following logic to the CameraSystem.
        // CameraSystem will set viewport and follow the player entity when available.
        if (auto camera_ptr = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            _camera_system.update(_registry, *camera_ptr, dt, viewport.x, viewport.y, _player_id);
        }

        _level.update(dt);

        handle_level_transitions();
    }


    void PlayState::handle_level_transitions() {
        if (LevelSystem::handle_transitions(_registry, _player_id, _level, _current_level_path, _level_transition_delay,
                                            0.016f)) {
            on_exit();
            on_enter();
        }
    }

    void PlayState::handle_input() {
        _game.input().poll();
        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _game.pop_state();
        }
    }

    // Renders the game world and HUD.
    void PlayState::render() {
        _game.renderer().begin_frame();

        auto camera_ptr = _level.camera();
        // Prepare a camera pointer (use a dummy camera when none is provided by the level)
        Camera dummy;
        // camera_ptr is a std::shared_ptr<Camera>; get a raw pointer for APIs that expect Camera*.
        Camera *cam = camera_ptr ? camera_ptr.get() : &dummy;
        if (camera_ptr) {
            _game.renderer().set_camera(cam->x(), cam->y());
        }

        // Render background(s)
        static thread_local std::vector<EntityID> bg_entities;
        _registry.get_entities_with<BackgroundComponent>(bg_entities);

        // Sorting backgrounds every frame is only needed if they change.
        // Keep it simple here.
        std::sort(bg_entities.begin(), bg_entities.end(), [&](EntityID a, EntityID b) {
            auto *bga = _registry.get_component<BackgroundComponent>(a);
            auto *bgb = _registry.get_component<BackgroundComponent>(b);
            return (bga && bgb) ? (bga->parallax < bgb->parallax) : false;
        });

        for (auto entity: bg_entities) {
            if (auto *bg = _registry.get_component<BackgroundComponent>(entity)) {
                _background_system.render(_game.renderer(), *cam, _game.assets(), *bg);
            }
        }
        _cloud_system.render(_game.renderer(), *cam, _game.assets(), _registry);

        _level.render(_game.renderer());

        // Render all entities that have a SpriteComponent. SpriteRenderSystem reads Position/Size/SpriteComponent and issues draw calls.
        _sprite_render_system.render(_game.renderer(), *cam, _registry);

        // Draw HUD
        std::string level_name = "Level 1";
        if (_current_level_path == mario::constants::LEVEL2_PATH) {
            level_name = "Level 2";
        }
        _hud.set_level_name(level_name);
        _hud.render();

        _game.renderer().end_frame();
    }

    bool PlayState::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario
