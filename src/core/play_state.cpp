#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"
#include "mario/util/Spawner.hpp"
#include "mario/world/TileMap.hpp"

#include <algorithm>
#include <string>

namespace mario {
    namespace {
        constexpr int BACKGROUND_TEXTURE_ID = 1000;
    }

    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0), _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {
    }

    void PlayState::on_enter() {
        // Load level
        _level.load(_current_level_path);

        // Load background texture if level specifies one
        const std::string &bg = _level.background_path();
        if (!bg.empty()) {
            if (_game.assets().load_texture(BACKGROUND_TEXTURE_ID, bg)) {
                // create background entity and attach component
                auto id = _registry.create_entity();
                BackgroundComponent bc;
                bc.texture_id = BACKGROUND_TEXTURE_ID;
                bc.preserve_aspect = true;
                // Use Fill so the background always covers the viewport (may crop), which effectively "zooms" small images
                bc.scale_mode = BackgroundComponent::ScaleMode::Fill;
                // Apply optional per-level background scale
                bc.scale_multiplier = _level.background_scale();
                bc.parallax = 0.0f; // Attached to camera
                bc.repeat = false; // Do not repeat if not seamless
                _registry.add_component(id, bc);
            }
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

    void PlayState::on_exit() {
        // Clear registry to remove entities/components from the previous level
        _registry.clear();
        _player_id = 0;
        _level.unload();
    }

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
        if (tile_map) {
            CollisionSystem::update(_registry, *tile_map, dt);
            LevelSystem::check_ground_status(_registry, *tile_map);
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

    void PlayState::render() {
        _game.renderer().begin_frame();

        if (auto camera = _level.camera()) {
            _game.renderer().set_camera(camera->x(), camera->y());
        }

        // Render background(s)
        auto bg_entities = _registry.get_entities_with<BackgroundComponent>();
        for (auto entity : bg_entities) {
            auto* bg = _registry.get_component<BackgroundComponent>(entity);
            if (!bg) continue;
            if (auto camera = _level.camera()) {
                _background_system.render(_game.renderer(), *camera, _game.assets(), *bg);
            } else {
                Camera dummy;
                _background_system.render(_game.renderer(), dummy, _game.assets(), *bg);
            }
        }

        _level.render(_game.renderer());

        // ECS Rendering
        auto renderables = _registry.get_entities_with<SpriteComponent>();
        for (auto entity : renderables) {
            auto* pos = _registry.get_component<PositionComponent>(entity);
            auto* size = _registry.get_component<SizeComponent>(entity);
            auto* sprite = _registry.get_component<SpriteComponent>(entity);

            if (!pos || !size) continue;

            if (sprite->shape == SpriteComponent::Shape::Rectangle) {
                _game.renderer().draw_rect(pos->x, pos->y, size->width, size->height, sprite->color);
            } else {
                _game.renderer().draw_ellipse(pos->x, pos->y, size->width, size->height, sprite->color);
            }
        }

        // Draw HUD
        std::string level_name = "Level 1";
        if (_current_level_path.find("level2") != std::string::npos) {
            level_name = "Level 2";
        }
        _hud.set_level_name(level_name);
        _hud.render();

        _game.renderer().end_frame();
    }

    bool PlayState::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario
