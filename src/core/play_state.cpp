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
#include <string>
#include <random>

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

    void PlayState::create_background_entity(int texture_id, bool preserve_aspect, BackgroundComponent::ScaleMode scale_mode, float scale_multiplier, float parallax, bool repeat, float offset_x, float offset_y) {
        auto id = _registry.create_entity();
        BackgroundComponent bc;
        bc.texture_id = texture_id;
        bc.preserve_aspect = preserve_aspect;
        bc.scale_mode = scale_mode;
        bc.scale_multiplier = scale_multiplier;
        bc.parallax = parallax;
        bc.repeat = repeat;
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
                auto id = _registry.create_entity();
                BackgroundComponent bc;
                bc.texture_id = BACKGROUND_TEXTURE_ID;
                bc.preserve_aspect = true;
                bc.scale_mode = BackgroundComponent::ScaleMode::Fill;
                bc.scale_multiplier = _level.background_scale();
                bc.parallax = 0.0f; 
                bc.repeat = false;
                bc.repeat_x = false;
                bc.offset_x = 0.0f;
                bc.offset_y = 0.0f;
                _registry.add_component(id, bc);
            }

            // Load additional background layers from level data
            int texture_id = BACKGROUND_TEXTURE_ID + 1;
            for (const auto& layer : _level.background_layers()) {
                if (_game.assets().load_texture(texture_id, layer.path)) {
                    auto id = _registry.create_entity();
                    BackgroundComponent bc;
                    bc.texture_id = texture_id;
                    bc.preserve_aspect = true;
                    bc.scale_mode = BackgroundComponent::ScaleMode::Fit;
                    bc.scale_multiplier = layer.scale;
                    bc.parallax = layer.parallax;
                    bc.repeat = layer.repeat;
                    bc.repeat_x = layer.repeat_x;
                    bc.offset_x = 0.0f;
                    bc.offset_y = 0.0f;
                    _registry.add_component(id, bc);
                }
                ++texture_id;
            }
        }

        // Load cloud textures
        const int CLOUD_BIG_ID = 2000;
        const int CLOUD_MEDIUM_ID = 2001;
        const int CLOUD_SMALL_ID = 2002;
        _game.assets().load_texture(CLOUD_BIG_ID, "assets/environment/background/cloud_big.png");
        _game.assets().load_texture(CLOUD_MEDIUM_ID, "assets/environment/background/cloud_medium.png");
        _game.assets().load_texture(CLOUD_SMALL_ID, "assets/environment/background/cloud_small.png");

        // Adjustable cloud spawn counts
        const int NUM_BIG_CLOUDS = 1;
        const int NUM_MEDIUM_CLOUDS = 2;
        const int NUM_SMALL_CLOUDS = 3;

        // Random Y positions
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> big_y_dist(40.0f, 60.0f);
        std::uniform_real_distribution<float> med_y_dist(60.0f, 80.0f);
        std::uniform_real_distribution<float> small_y_dist(80.0f, 100.0f);

        // Create cloud entities
        // Big clouds
        for (int i = 0; i < NUM_BIG_CLOUDS; ++i) {
            auto id = _registry.create_entity();
            CloudComponent cc;
            cc.texture_id = CLOUD_BIG_ID;
            cc.layer = CloudComponent::Layer::Big;
            cc.speed = 40.0f;
            cc.x = static_cast<float>(-300 - i * 300);
            cc.y = big_y_dist(gen);
            cc.scale = 1.0f;
            _registry.add_component(id, cc);
        }
        // Medium clouds
        for (int i = 0; i < NUM_MEDIUM_CLOUDS; ++i) {
            auto id = _registry.create_entity();
            CloudComponent cc;
            cc.texture_id = CLOUD_MEDIUM_ID;
            cc.layer = CloudComponent::Layer::Medium;
            cc.speed = 30.0f;
            cc.x = static_cast<float>(-300 - i * 250);
            cc.y = med_y_dist(gen);
            cc.scale = 1.0f;
            _registry.add_component(id, cc);
        }
        // Small clouds
        for (int i = 0; i < NUM_SMALL_CLOUDS; ++i) {
            auto id = _registry.create_entity();
            CloudComponent cc;
            cc.texture_id = CLOUD_SMALL_ID;
            cc.layer = CloudComponent::Layer::Small;
            cc.speed = 20.0f;
            cc.x = static_cast<float>(-300 - i * 200);
            cc.y = small_y_dist(gen);
            cc.scale = 1.0f;
            _registry.add_component(id, cc);
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

    // Renders the game world and HUD.
    void PlayState::render() {
        _game.renderer().begin_frame();

        if (auto camera = _level.camera()) {
            _game.renderer().set_camera(camera->x(), camera->y());
        }

        // Render background(s)
        static thread_local std::vector<EntityID> bg_entities;
        _registry.get_entities_with<BackgroundComponent>(bg_entities);

        // Sort by parallax (ascending) so background layers (low parallax) are drawn first
        std::sort(bg_entities.begin(), bg_entities.end(), [&](EntityID a, EntityID b) {
            auto* bga = _registry.get_component<BackgroundComponent>(a);
            auto* bgb = _registry.get_component<BackgroundComponent>(b);
            if (!bga || !bgb) return false;
            return bga->parallax < bgb->parallax;
        });

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

        // Render clouds
        if (auto camera = _level.camera()) {
            _cloud_system.render(_game.renderer(), *camera, _game.assets(), _registry);
        } else {
            Camera dummy;
            _cloud_system.render(_game.renderer(), dummy, _game.assets(), _registry);
        }

        _level.render(_game.renderer());

        // ECS Rendering
        static thread_local std::vector<EntityID> renderables;
        _registry.get_entities_with<SpriteComponent>(renderables);
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
