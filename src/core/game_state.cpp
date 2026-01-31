#include "mario/core/GameState.hpp"
#include "mario/entities/Enemy.hpp"
#include "mario/world/Camera.hpp"

#include <algorithm>
#include <cctype>
#include <string>

// Helpers
namespace {
    // Cast to lowercase
    std::string to_lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    // Create entities from spawn data with a type and position using a factory pattern
    std::unique_ptr<mario::Entity> create_entity_for_spawn(const mario::EntitySpawn &spawn, float tile_size) {
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
}

namespace mario {
    void PlayState::on_enter() {

        _entities.clear();
        // Initialize player position
        _player.set_position(32.0f, 32.0f);
        // Load level
        _level.load("assets/levels/demo.json");

        // Check if the player entity belongs to the level and is spawned
        bool player_spawned = false;
        // Looking at the level tile map
        if (auto tile_map = _level.tile_map()) {
            // Getting the tile size
            const float tile_size = static_cast<float>(tile_map->tile_size());
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
            const auto viewport = _renderer.viewport_size();
            camera->set_viewport(viewport.x, viewport.y);
            camera->set_target(_player.x() + _player.width() * 0.5f,
                               _player.y() + _player.height() * 0.5f);
            camera->update(0.0f);
        }
        
        _running = true;
    }

    void PlayState::on_exit() {
        _level.unload();
        _entities.clear();
    }

    void PlayState::update(float dt) {
        _input.poll();
        float axis = 0.0f;
        if (_input.is_pressed(InputManager::Action::MoveLeft)) axis -= 1.0f;
        if (_input.is_pressed(InputManager::Action::MoveRight)) axis += 1.0f;

        // Set move axis i.e., direction of movement
        _player.set_move_axis(axis);
        // Check if the player is jumping and the jump button is pressed
        _player.set_jump_pressed(_input.is_pressed(InputManager::Action::Jump));
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
            const auto viewport = _renderer.viewport_size();
            camera->set_viewport(viewport.x, viewport.y);
            camera->set_target(_player.x() + _player.width() * 0.5f,
                               _player.y() + _player.height() * 0.5f);
        }
        _level.update(dt);

        // Exit the game if the escape key is pressed
        if (_input.is_pressed(InputManager::Action::Escape)) {
            _running = false;
        }

        // Reset the level if the player falls below the map
        if (tile_map) {
            float map_bottom = static_cast<float>(tile_map->height() * tile_map->tile_size());
            if (_player.y() > map_bottom) {
                on_exit();
                on_enter();
            }
        }
    }

    void PlayState::render() {
        _renderer.begin_frame();
        
        if (auto camera = _level.camera()) {
            _renderer.set_camera(camera->x(), camera->y());
        }

        _level.render(_renderer);

        _player.render(_renderer);

        for (auto &entity: _entities) {
            entity->render(_renderer);
        }

        _renderer.end_frame();
    }

    bool PlayState::is_running() const { return _running && _renderer.is_open(); }

    void MenuState::on_enter() {
    }

    void MenuState::on_exit() {
    }

    void MenuState::update(float dt) { (void) dt; }

    void MenuState::render() {
    }

    void PauseState::on_enter() {
    }

    void PauseState::on_exit() {
    }

    void PauseState::update(float dt) { (void) dt; }

    void PauseState::render() {
    }
} // namespace mario
