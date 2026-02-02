#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/Position.hpp"
#include "mario/ecs/components/Velocity.hpp"
#include "mario/ecs/components/Size.hpp"
#include "mario/ecs/components/PlayerInput.hpp"
#include "mario/ecs/components/JumpState.hpp"
#include "mario/ecs/components/PlayerStats.hpp"
#include "mario/ecs/components/Type.hpp"
#include "mario/ecs/components/CollisionInfo.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/Sprite.hpp"
#include "mario/util/Spawner.hpp"
#include "mario/entities/PlayerConstants.hpp"

#include <cmath>
#include <algorithm>
#include <cctype>
#include <string>

namespace mario {
    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0), _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {
    }

    void PlayState::on_enter() {
        // Load level
        _level.load(_current_level_path);

        // Spawn entities
        bool player_spawned = false;
        if (const auto tile_map = _level.tile_map()) {
            const auto tile_size = static_cast<float>(tile_map->tile_size());
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

        // Initialize camera target
        update_camera();
        if (auto camera = _level.camera()) {
            camera->update(0.0f);
        }

        _running = true;
        _level_transition_delay = 0.5f; // LevelTransitionCooldown
    }

    void PlayState::on_exit() {
        _level.unload();
    }

    void PlayState::update(float dt) {
        handle_input();

        auto tile_map = _level.tile_map();

        // Update ECS systems
        _player_input.update(_registry, _game.input());
        _player_movement.update(_registry, dt);
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
            auto* pos = _registry.get_component<Position>(_player_id);
            auto* size = _registry.get_component<Size>(_player_id);
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

        _level.render(_game.renderer());

        // ECS Rendering
        auto renderables = _registry.get_entities_with<Sprite>();
        for (auto entity : renderables) {
            auto* pos = _registry.get_component<Position>(entity);
            auto* size = _registry.get_component<Size>(entity);
            auto* sprite = _registry.get_component<Sprite>(entity);

            if (!pos || !size) continue;

            if (sprite->shape == Sprite::Shape::Rectangle) {
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
