// Implements the PlayState class, which manages the main gameplay state, including level loading, player spawning, and HUD updates.
// Handles entering and exiting the play state, updating game logic, and rendering the game world and HUD.

#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"
#include "mario/ecs/components/CloudComponent.hpp"
#include "mario/util/Spawner.hpp"
#include "mario/world/TileMap.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace mario {
    namespace {
        constexpr int BACKGROUND_TEXTURE_ID = 1000;
    }

    // Constructor initializes the PlayState with a reference to the game and optional level path.
    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0), _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {
    }

    void PlayState::create_background_entity(int texture_id, bool preserve_aspect, BackgroundComponent::ScaleMode scale_mode, float scale_multiplier, float parallax, bool repeat, bool repeat_x, float offset_x, float offset_y) {
        auto id = _registry.create_entity();
        BackgroundComponent bc;
        bc.texture_id = texture_id;
        bc.preserve_aspect = preserve_aspect;
        bc.scale_mode = scale_mode;
        bc.scale_multiplier = scale_multiplier;
        bc.parallax = parallax;
        bc.repeat = repeat;
        bc.repeat_x = repeat_x;
        bc.offset_x = offset_x;
        bc.offset_y = offset_y;
        _registry.add_component(id, bc);
    }

    // Called when entering the play state. Loads the level, background, and spawns the player.
    void PlayState::on_enter() {

        // Load level
        _level.load(_current_level_path);

        // Background loading (level dependent)
        const std::string& level_bg_path = _level.background_path();
        if (!level_bg_path.empty()) {
            if (_game.assets().load_texture(BACKGROUND_TEXTURE_ID, level_bg_path)) {
                // Main background entity
                create_background_entity(BACKGROUND_TEXTURE_ID, true, BackgroundComponent::ScaleMode::Fill, _level.background_scale(), 0.0f, false, false, 0.0f, 0.0f);
            }

            // Load additional background layers from level data
            int texture_id = BACKGROUND_TEXTURE_ID + 1;
            for (const auto& layer : _level.background_layers()) {
                if (_game.assets().load_texture(texture_id, layer.path)) {
                    create_background_entity(texture_id, true, BackgroundComponent::ScaleMode::Fit, layer.scale, layer.parallax, layer.repeat, layer.repeat_x, 0.0f, 0.0f);
                }
                ++texture_id;
            }
        }

        // Initialize clouds if enabled for this level
        if (_level.clouds_enabled()) {
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

        // Initialize camera
        update_camera();
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            camera->set_viewport(viewport.x, viewport.y);
            
            auto* pos = _registry.get_component<PositionComponent>(_player_id);
            auto* size = _registry.get_component<SizeComponent>(_player_id);
            if (pos && size) {
                const float center_x = pos->x + size->width * 0.5f;
                const float center_y = pos->y + size->height * 0.5f;
                camera->set_target(center_x, center_y);
                
                // Start with an offset from the target to make the damping animation visible
                const float target_x = center_x - viewport.x * 0.5f;
                const float target_y = center_y - viewport.y * 0.5f;
                camera->set_position(target_x - 100.0f, target_y);
            }
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

        update_camera();
        _level.update(dt);

        handle_level_transitions();
    }

    void PlayState::update_camera() {
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            camera->set_viewport(viewport.x, viewport.y);
            auto* pos = _registry.get_component<PositionComponent>(_player_id);
            auto* size = _registry.get_component<SizeComponent>(_player_id);
            if (pos && size) {
                camera->set_target(pos->x + size->width * 0.5f,
                                   pos->y + size->height * 0.5f);
            }
        }
    }

    void PlayState::handle_level_transitions() {
        if (LevelSystem::handle_transitions(_registry, _player_id, _level, _current_level_path, _level_transition_delay, 0.016f)) {
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
        Camera* cam = camera_ptr ? camera_ptr.get() : &dummy;
        if (camera_ptr) {
            _game.renderer().set_camera(cam->x(), cam->y());
        }

        // Render background(s)
        static thread_local std::vector<EntityID> bg_entities;
        _registry.get_entities_with<BackgroundComponent>(bg_entities);

        // Sorting backgrounds every frame is only needed if they change.
        // Keep it simple here.
        std::sort(bg_entities.begin(), bg_entities.end(), [&](EntityID a, EntityID b) {
            auto* bga = _registry.get_component<BackgroundComponent>(a);
            auto* bgb = _registry.get_component<BackgroundComponent>(b);
            return (bga && bgb) ? (bga->parallax < bgb->parallax) : false;
        });

        for (auto entity : bg_entities) {
            if (auto* bg = _registry.get_component<BackgroundComponent>(entity)) {
                _background_system.render(_game.renderer(), *cam, _game.assets(), *bg);
            }
        }
        _cloud_system.render(_game.renderer(), *cam, _game.assets(), _registry);

        _level.render(_game.renderer());

        // ECS Rendering - combined components fetch
        static thread_local std::vector<EntityID> renderables;
        _registry.get_entities_with<SpriteComponent>(renderables);
        for (auto entity : renderables) {
            auto* pos = _registry.get_component<PositionComponent>(entity);
            auto* size = _registry.get_component<SizeComponent>(entity);
            auto* sprite = _registry.get_component<SpriteComponent>(entity);

            if (pos && size && sprite) {
                if (sprite->shape == SpriteComponent::Shape::Rectangle) {
                    _game.renderer().draw_rect(pos->x, pos->y, size->width, size->height, sprite->color);
                } else {
                    _game.renderer().draw_ellipse(pos->x, pos->y, size->width, size->height, sprite->color);
                }
            }
        }

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
