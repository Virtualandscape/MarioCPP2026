#include "mario/core/GameState.hpp"
#include "mario/world/Camera.hpp"

namespace mario {
    void PlayState::on_enter() {
        // Initialize player position
        _player.set_position(32.0f, 32.0f);
        
        // Load level
        _level.load("assets/levels/demo.json");
        
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
