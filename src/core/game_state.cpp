#include "mario/core/GameState.hpp"
#include "mario/core/Game.hpp"
#include "mario/entities/Enemy.hpp"
#include "mario/world/Camera.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <SFML/Window/Keyboard.hpp>

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
    constexpr float LevelTransitionCooldown = 0.5f;
}

namespace mario {
    PlayState::PlayState(Game& game) : _game(game) {}
    PlayState::PlayState(Game& game, std::string level_path) : _game(game), _current_level_path(std::move(level_path)) {}

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
                float map_right = static_cast<float>(tile_map->width() * tile_map->tile_size());
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
        _game.renderer().draw_text(level_name, 10, 10, 24, sf::Color::White);

        _game.renderer().end_frame();
    }

    bool PlayState::is_running() const { return _running && _game.renderer().is_open(); }

    MenuState::MenuState(Game& game) : _game(game) {
        _levels = {"assets/levels/level1.json", "assets/levels/level2.json"};
    }

    void MenuState::on_enter() {
        _running = true;
    }

    void MenuState::on_exit() {
    }

    void MenuState::update(float dt) {
        (void)dt;
        _game.input().poll();

        bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
        bool enter = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        if (up && !_up_pressed) {
            _selected_index = (_selected_index - 1 + static_cast<int>(_levels.size())) % static_cast<int>(_levels.size());
        }
        if (down && !_down_pressed) {
            _selected_index = (_selected_index + 1) % static_cast<int>(_levels.size());
        }
        if (enter && !_enter_pressed) {
            _game.push_state(std::make_shared<PlayState>(_game, _levels[_selected_index]));
            return;
        }

        _up_pressed = up;
        _down_pressed = down;
        _enter_pressed = enter;

        // Mouse handling
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(_game.renderer().window());
        for (size_t i = 0; i < _levels.size(); ++i) {
            float x = 300;
            float y = 150 + static_cast<float>(i) * 100;
            float w = 200;
            float h = 50;

            if (mouse_pos.x >= x && mouse_pos.x <= x + w && mouse_pos.y >= y && mouse_pos.y <= y + h) {
                _selected_index = static_cast<int>(i);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    _game.push_state(std::make_shared<PlayState>(_game, _levels[_selected_index]));
                    return;
                }
            }
        }

        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _running = false;
        }
    }

    void MenuState::render() {
        _game.renderer().begin_frame();

        // Draw background
        _game.renderer().draw_rect(0, 0, 800, 480, sf::Color(50, 50, 50));

        // Draw level options as rectangles and text
        for (size_t i = 0; i < _levels.size(); ++i) {
            sf::Color color = (static_cast<int>(i) == _selected_index) ? sf::Color::Yellow : sf::Color::White;
            float x = 300;
            float y = 150 + static_cast<float>(i) * 100;
            _game.renderer().draw_rect(x, y, 200, 50, color);
            
            std::string label = "Level " + std::to_string(i + 1);
            _game.renderer().draw_text(label, x + 50, y + 10, 24, sf::Color::Black);

            // Draw a small indicator for selection
            if (static_cast<int>(i) == _selected_index) {
                _game.renderer().draw_rect(270, y + 10, 20, 30, sf::Color::Red);
            }
        }

        _game.renderer().end_frame();
    }

    bool MenuState::is_running() const { return _running && _game.renderer().is_open(); }

    void PauseState::on_enter() {
    }

    void PauseState::on_exit() {
    }

    void PauseState::update(float dt) { (void) dt; }

    void PauseState::render() {
    }
} // namespace mario
