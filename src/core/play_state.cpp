#include "mario/core/PlayState.hpp"
#include "mario/core/Game.hpp"
#include "mario/entities/Enemy.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/Position.hpp"
#include "mario/ecs/components/Velocity.hpp"
#include "mario/ecs/components/Size.hpp"
#include "mario/ecs/components/PlayerInput.hpp"
#include "mario/ecs/components/JumpState.hpp"
#include "mario/ecs/components/PlayerStats.hpp"
#include "mario/ecs/components/Type.hpp"
#include "mario/ecs/components/CollisionInfo.hpp"
#include "mario/entities/PlayerConstants.hpp"

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
        const float pos_x = static_cast<float>(spawn.tile_x) * tile_size;
        const float pos_y = static_cast<float>(spawn.tile_y) * tile_size;

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
    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {
    }

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0), _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {
    }

    void PlayState::on_enter() {
        _entities.clear();
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
                for (const auto &spawn: _level.entity_spawns()) {
                    const auto type = to_lower(spawn.type);
                    // If the entity type is player
                    if (type == "player") {
                        player_spawned = true;
                        // Create the ECS player
                        _player_id = _registry.create_entity();
                        _registry.add_component<Position>(_player_id, {
                                                              static_cast<float>(spawn.tile_x) * tile_size, static_cast<float>(spawn.tile_y) * tile_size
                                                          });
                        _registry.add_component<Velocity>(_player_id, {0.0f, 0.0f});
                        // Use the real player sprite/collision size (pixels)
                        _registry.add_component<Size>(_player_id, {player::Width, player::Height});
                        _registry.add_component<PlayerInput>(_player_id, {});
                        _registry.add_component<JumpState>(_player_id, {});
                        _registry.add_component<PlayerStats>(_player_id, {});
                        _registry.add_component<Type>(_player_id, {EntityType::Player});
                        _registry.add_component<CollisionInfo>(_player_id, {});
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
            // Create the ECS player
            _player_id = _registry.create_entity();
            _registry.add_component<Position>(_player_id, {32.0f, 32.0f});
            _registry.add_component<Velocity>(_player_id, {0.0f, 0.0f});
            // Use the real player sprite/collision size (pixels)
            _registry.add_component<Size>(_player_id, {player::Width, player::Height});
            _registry.add_component<PlayerInput>(_player_id, {});
            _registry.add_component<JumpState>(_player_id, {});
            _registry.add_component<PlayerStats>(_player_id, {});
            _registry.add_component<Type>(_player_id, {EntityType::Player});
            _registry.add_component<CollisionInfo>(_player_id, {});
            // Sync player position for rendering
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

        auto tile_map = _level.tile_map();

        // Update ECS systems for player
        _player_input.update(_registry, _game.input());
        _player_movement.update(_registry, dt);
        _physics.update(_registry, dt);
        if (tile_map) {
            CollisionSystem::update(_registry, *tile_map, dt);
        }

        // Sync player position from ECS
        auto *pos = _registry.get_component<Position>(_player_id);
        if (pos) {
            _player.set_position(pos->x, pos->y);
        }
        // Also sync velocity from ECS so legacy collision resolution has correct velocity
        auto *vel = _registry.get_component<Velocity>(_player_id);
        if (vel) {
            _player.set_velocity(vel->vx, vel->vy);
        }

        // Update entities and physics for legacy (non-ECS) entities
        for (auto &entity: _entities) {
            // Update logic for legacy entities
            entity->update(dt);
            // Update physics and collision for each legacy entity
            _physics.update(*entity, dt);
            if (tile_map) {
                CollisionSystem::check_entity_collision(*entity, *tile_map, dt);
            }
        }

        // Check for entity vs entity collisions (Player vs Enemies) for legacy entities
        for (auto &entity : _entities) {
            CollisionSystem::check_entity_vs_entity_collision(_player, *entity, dt);
        }

        // Sync resolved player position/velocity back into the ECS so physics doesn't overwrite it next frame
        {
            auto *pos_comp = _registry.get_component<Position>(_player_id);
            auto *vel_comp = _registry.get_component<Velocity>(_player_id);
            if (pos_comp) {
                pos_comp->x = _player.x();
                pos_comp->y = _player.y();
            }
            if (vel_comp) {
                vel_comp->vx = _player.vx();
                vel_comp->vy = _player.vy();
            }
        }

        // Reset jump if player is on ground
        if (tile_map && _player.is_on_ground(*tile_map)) {
            auto jump = _registry.get_component<JumpState>(_player_id);
            if (jump) jump->jump_count = 0;
        }

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
            float map_bottom = static_cast<float>(tile_map->height()) * static_cast<float>(tile_map->tile_size());
            if (_player.y() > map_bottom) {
                on_exit();
                on_enter();
                return;
            }

            if (_level_transition_delay <= 0.0f) {
                // Load next level if the player reaches the right end
                const int map_right_px = tile_map->width() * tile_map->tile_size();
                if (_player.x() + _player.width() > static_cast<float>(map_right_px)) {
                    if (_current_level_path.find("level1") != std::string::npos) {
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

        // Removed debug draw (AABB and collision-coloring) so player and enemies render via their own render methods

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
