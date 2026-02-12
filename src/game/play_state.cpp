// Implements the PlayState class, which manages the main gameplay state, including level loading, player spawning, and HUD updates.
// Handles entering and exiting the play state, updating game logic, and rendering the game world and HUD.

#include "mario/game/PlayState.hpp"
#include "mario/game/Game.hpp"
#include "mario/world/World.hpp"
#include "mario/helpers/Spawner.hpp"
#include "mario/helpers/Constants.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/systems/PhysicsSystems.hpp"
#include "mario/systems/RenderSystems.hpp"
#include "mario/systems/GameplaySystems.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace mario {

    PlayState::PlayState(Game &game) : _game(game), _player_id(0), _hud(game.renderer()) {}

    PlayState::PlayState(Game &game, std::string level_path) : _game(game), _player_id(0),
                                                               _current_level_path(std::move(level_path)),
                                                               _hud(game.renderer()) {}

    void PlayState::on_enter() {
        _level.load(_current_level_path);
        auto& registry = _game.entity_manager();

        const std::string &level_bg_path = _level.background_path();
        if (!level_bg_path.empty()) {
            if (_game.assets().load_texture(mario::constants::BACKGROUND_TEXTURE_ID, level_bg_path)) {
                EntityID bg_id = registry.create_entity();
                BackgroundComponent bg;
                bg.texture_id = mario::constants::BACKGROUND_TEXTURE_ID;
                bg.scale_mode = BackgroundComponent::ScaleMode::Fill;
                bg.scale_multiplier = _level.background_scale();
                registry.add_component(bg_id, bg);
            }

            int texture_id = mario::constants::BACKGROUND_TEXTURE_ID + 1;
            for (const auto &layer: _level.background_layers()) {
                if (_game.assets().load_texture(texture_id, layer.path)) {
                    EntityID layer_id = registry.create_entity();
                    BackgroundComponent bg;
                    bg.texture_id = texture_id;
                    bg.scale_mode = BackgroundComponent::ScaleMode::Fit;
                    bg.scale_multiplier = layer.scale;
                    bg.parallax = layer.parallax;
                    bg.repeat = layer.repeat;
                    bg.repeat_x = layer.repeat_x;
                    registry.add_component(layer_id, bg);
                }
                ++texture_id;
            }
        }

        if (_level.clouds_enabled()) {
            Spawner::spawn_clouds(registry, _game.assets());
        }

        bool player_spawned = false;
        if (const auto tile_map = _level.tile_map()) {
            const auto tile_size = static_cast<float>(tile_map->tile_size());
            if (tile_size > 0.0f) {
                for (const auto &spawn: _level.entity_spawns()) {
                    if (spawn.type == "player" || spawn.type == "Player") {
                        _player_id = Spawner::spawn_player(registry, spawn, _game.assets());
                        player_spawned = true;
                    } else {
                        Spawner::spawn_enemy(registry, spawn);
                    }
                }
            }
        }
        if (!player_spawned) {
            _player_id = Spawner::spawn_player_default(registry, _game.assets());
        }

        if (auto camera = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            systems::render::init_camera(registry, *camera, viewport.x, viewport.y, _player_id, -100.0f, 0.0f);
        }

        _running = true;
        _level_transition_delay = 0.5f;
        _game.system_scheduler().clear();
        setup_systems();
    }

    void PlayState::on_exit() {
        _game.entity_manager().clear();
        _player_id = 0;
        _level.unload();
    }

    void PlayState::update(float dt) {
        handle_input();
        auto& registry = _game.entity_manager();
        _game.system_scheduler().update(registry, dt);

        if (auto camera_ptr = _level.camera()) {
            const auto viewport = _game.renderer().viewport_size();
            systems::render::update_camera(registry, *camera_ptr, dt, viewport.x, viewport.y, _player_id);
        }

        _level.update(dt);
        handle_level_transitions();
    }

    void PlayState::handle_level_transitions() {
        if (_level_transition_pending) {
            _level_transition_pending = false;
            on_exit();
            on_enter();
        }
    }

    void PlayState::handle_input() {
        _game.input().poll();
        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _game.pop_state();
        }

        bool current = _game.input().is_pressed(InputManager::Action::ToggleDebug);
        if (current && !_debug_toggle_last_state) {
            _game.renderer().toggle_debug_bboxes();
        }
        _debug_toggle_last_state = current;
    }

    void PlayState::setup_systems() {
        auto& scheduler = _game.system_scheduler();

        scheduler.add_system([this](EntityManager& registry, float dt) {
            systems::gameplay::handle_player_input(registry, _game.input());
        });
        scheduler.add_system([](EntityManager& registry, float dt) {
            systems::gameplay::update_player_movement(registry, dt);
        });
        scheduler.add_system([](EntityManager& registry, float dt) {
            systems::render::update_animations(registry, dt);
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                systems::gameplay::update_enemies(registry, *tile_map, dt);
            }
        });
        scheduler.add_system([](EntityManager& registry, float dt) {
            systems::physics::update(registry, dt);
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            if (const auto tile_map = _level.tile_map()) {
                systems::physics::resolve_collisions(registry, *tile_map, dt);
            }
        });
        scheduler.add_system([this](EntityManager& registry, float dt) {
            if (systems::gameplay::check_level_transitions(registry, _player_id, _level, _current_level_path, _level_transition_delay, dt)) {
                _level_transition_pending = true;
            }
        });

        scheduler.add_render_system([this](EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera) {
            systems::render::render_backgrounds(registry, renderer, assets, camera);
            systems::render::render_clouds(renderer, camera, assets, registry);
            _level.render(renderer);
            systems::render::render_sprites(renderer, camera, registry, assets);
            systems::render::render_debug(registry, renderer, camera);

            std::string level_name = (_current_level_path == mario::constants::LEVEL2_PATH) ? "Level 2" : "Level 1";
            _hud.set_level_name(level_name);
            _hud.render();
        });
    }

    void PlayState::render() {
        _game.renderer().begin_frame();
        auto camera_ptr = _level.camera();
        Camera dummy;
        Camera *cam = camera_ptr ? camera_ptr.get() : &dummy;
        _game.system_scheduler().render(_game.entity_manager(), _game.renderer(), _game.assets(), *cam);

        _game.renderer().end_frame();
    }

    bool PlayState::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario
