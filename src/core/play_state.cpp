#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/entities/Enemy.hpp"
#include "mario/world/Camera.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace {
    // Cast to lowercase
    std::string to_lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    // Create entities from spawn data with a type and position using a factory pattern
    std::unique_ptr<mario::Entity> create_entity_for_spawn(const mario::EntitySpawn &spawn, const float tile_size) {
        const std::string type = to_lower(spawn.type);
        const float pos_x = spawn.tile_x * tile_size;
        const float pos_y = spawn.tile_y * tile_size;

        if (type == "goomba") {
            auto entity = std::make_unique<mario::Goomba>();
            entity->set_position(pos_x, pos_y);
            return entity;
        }

        if (type == "koopa") {
            auto entity = std::make_unique<mario::Koopa>();
            entity->set_position(pos_x, pos_y);
            return entity;
        }

        return nullptr;
    }
    constexpr float LevelTransitionCooldown = 0.5f;
}

namespace mario {
    PlayState::PlayState(Game& game) : _game(game), _hud(game.renderer()) {}
    PlayState::PlayState(Game& game, std::string level_path) : _game(game), _current_level_path(std::move(level_path)), _hud(game.renderer()) {}

    void PlayState::on_enter() {
        _entities.clear();
        _player.set_move_axis(0.0f);
        _player.set_velocity(0.0f, 0.0f);
        _player.reset_jump();
        // Initialize player position
        _player.set_position(32.0f, 32.0f);
        // Load level
        _level.load(_current_level_path);

        // Check if the player entity belongs to the level and is spawned
        bool player_spawned = false;
        // Looking at the level tile map
        if (const auto tile_map = _level.tile_map()) {
            // Getting the tile size
            const auto tile_size = static_cast<float>(tile_map->tile_size());
            if (tile_size > 0.0f) {
                // For each entity spawn in the level
                for (const auto &spawn : _level.entity_spawns()) {
                    const auto type = to_lower(spawn.type);
                    // If the entity type is player
                    if (type == "player") {
                        // Set player position based on spawn coordinates
                        _player.set_position(spawn.tile_x * tile_size, spawn.tile_y * tile_size);
                        player_spawned = true;
                        continue;
                    }
                    // If entity creation succeeds
                    if (auto entity = create_entity_for_spawn(spawn, tile_size)) {
                        // Insert the entity into the entities list
                        _entities.push_back(std::move(entity));
                    }
                }
            }
        }
        // If no player entity is found, set its default position
        if (!player_spawned) {
            _player.set_position(32.0f, 32.0f);
        }
        
        // Initialize camera target
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            camera->set_viewport(viewport.x, viewport.y);
            camera->set_target(_player.x() + _player.width() * 0.5f,
                               _player.y() + _player.height() * 0.5f);
            camera->update(0.0f);
        }
        
        _running = true;
        _level_transition_delay = LevelTransitionCooldown;
    }

    void PlayState::on_exit() {
        _level.unload();
        _entities.clear();
    }

    void PlayState::update(float dt) {
        _game.input().poll();
        if (_level_transition_delay > 0.0f) {
            _level_transition_delay = std::max(0.0f, _level_transition_delay - dt);
        }
        float axis = 0.0f;
        if (_game.input().is_pressed(InputManager::Action::MoveLeft)) axis -= 1.0f;
        if (_game.input().is_pressed(InputManager::Action::MoveRight)) axis += 1.0f;

        // Set move axis i.e., direction of movement
        _player.set_move_axis(axis);
        // Check if the player is jumping and the jump button is pressed
        _player.set_jump_pressed(_game.input().is_pressed(InputManager::Action::Jump));
        // Player velocity update and double jump handling
        _player.handle_input();
        
        auto tile_map = _level.tile_map();
        
        _physics.update(_player, dt);
        if (tile_map) {
            _collision.check_entity_collision(_player, *tile_map, dt);
        }
        
        // Update entities and physics
        for (auto &entity: _entities) {
            // Does nothing for the moment
            entity->update(dt);
            // Update physics and collision for each entity
            _physics.update(*entity, dt);
            if (tile_map) {
                _collision.check_entity_collision(*entity, *tile_map, dt);
            }
        }

        // Check for entity vs entity collisions (Player vs Enemies)
        for (auto &entity : _entities) {
            _collision.check_entity_vs_entity_collision(_player, *entity);
        }

        // Reset jump if player is on ground
        if (tile_map && _player.is_on_ground(*tile_map)) _player.reset_jump();

        // Handle camera movement and target for scrolling effect
        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            camera->set_viewport(viewport.x, viewport.y);
            camera->set_target(_player.x() + _player.width() * 0.5f,
                               _player.y() + _player.height() * 0.5f);
        }
        _level.update(dt);

        // Exit the game if the escape key is pressed
        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _game.pop_state();
            return;
        }

        // Reset the level if the player falls below the map
        if (tile_map) {
            float map_bottom = static_cast<float>(tile_map->height() * tile_map->tile_size());
            if (_player.y() > map_bottom) {
                on_exit();
                on_enter();
                return;
            }

            if (_level_transition_delay <= 0.0f) {
                // Load next level if the player reaches the right end
                auto map_right = static_cast<float>(tile_map->width() * tile_map->tile_size());
                if (_player.x() + _player.width() > map_right) {
                    if (_current_level_path == "assets/levels/level1.json") {
                        _current_level_path = "assets/levels/level2.json";
                    } else {
                        // Loop back to level 1 for now if we reach the end of level 2
                        _current_level_path = "assets/levels/level1.json";
                    }
                    on_exit();
                    on_enter();
                    return;
                }
            }
        }
    }

    void PlayState::render() {
        _game.renderer().begin_frame();
        
        if (auto camera = _level.camera()) {
            _game.renderer().set_camera(camera->x(), camera->y());
        }

        _level.render(_game.renderer());
        _player.render(_game.renderer());
        for (auto &entity: _entities) {
            entity->render(_game.renderer());
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
