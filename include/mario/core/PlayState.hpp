#pragma once

#include "mario/core/GameState.hpp"
#include "mario/systems/PhysicsSystem.hpp"
#include "mario/systems/PlayerControllerSystem.hpp"
#include "mario/systems/EnemySystem.hpp"
#include "mario/systems/AnimationSystem.hpp"
#include "mario/systems/LevelSystem.hpp"
#include "mario/systems/BackgroundSystem.hpp"
#include "mario/systems/CloudSystem.hpp"
#include "mario/systems/SpriteRenderSystem.hpp"
#include "mario/systems/CameraSystem.hpp"
#include "mario/systems/DebugDrawSystem.hpp"
#include "mario/world/Level.hpp"
#include "mario/ui/HUD.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/helpers/Constants.hpp"

#include <string>
#include <functional>
#include <vector>
#include <future>

namespace mario {
    class Game;

    class PlayState : public GameState {
    public:
        PlayState(Game &game);

        PlayState(Game &game, std::string level_path);

        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;

        bool is_running() const override;

    private:
        void handle_level_transitions();

        void handle_input();

        void setup_systems();

        void run_update_systems(EntityManager &registry, float dt);

        void run_render_systems(EntityManager &registry, const Camera &camera);

        Game &_game;
        EntityID _player_id;
        PhysicsSystem _physics;
        PlayerControllerSystem _player_controller;
        EnemySystem _enemy_system;
        AnimationSystem _animation_system;
        BackgroundSystem _background_system;
        CloudSystem _cloud_system;
        SpriteRenderSystem _sprite_render_system;
        CameraSystem _camera_system;
        DebugDrawSystem _debug_draw_system;
        Level _level;
        HUD _hud;
        bool _running = true;
        float _level_transition_delay = 0.0f;
        bool _level_transition_pending = false;

        // Initialize current level path using constants so the default matches the first level.
        std::string _current_level_path = std::string(mario::constants::LEVEL1_PATH);

        // Track the previous state of the ToggleDebug key to perform a rising-edge toggle
        bool _debug_toggle_last_state = false;

        // Ordered update callbacks to keep the ECS steps deterministic.
        std::vector<std::function<void(EntityManager&, float)>> _update_systems;
        // Render callbacks that rely on the camera context provided each frame.
        std::vector<std::function<void(EntityManager&, Renderer&, AssetManager&, const Camera&)>> _render_systems;

        // Async asset loading future and flag. When true, background decode is running and update() will finalize textures each frame.
        std::future<void> _asset_loading_future;
        bool _assets_loading = false;
    };
} // namespace mario
