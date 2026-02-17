#pragma once

#include "Zia/engine/Scene.hpp"
#include "Zia/game/systems/PhysicsSystem.hpp"
#include "Zia/game/systems/PlayerControllerSystem.hpp"
#include "Zia/game/systems/EnemySystem.hpp"
#include "Zia/game/systems/AnimationSystem.hpp"
#include "Zia/game/systems/LevelSystem.hpp"
#include "Zia/game/systems/BackgroundSystem.hpp"
#include "Zia/game/systems/CloudSystem.hpp"
#include "Zia/game/systems/SpriteRenderSystem.hpp"
#include "Zia/game/systems/CameraSystem.hpp"
#include "Zia/game/systems/DebugDrawSystem.hpp"
#include "Zia/game/systems/InspectorSystem.hpp"
#include "Zia/game/world/Level.hpp"
#include "Zia/game/ui/HUD.hpp"
#include "Zia/engine/IEntityManager.hpp"
#include "Zia/game/helpers/Constants.hpp"

#include <string>
#include <functional>
#include <vector>
#include <future>

namespace zia {
    class Game;

    // Scene managing active gameplay: loads levels, spawns entities and runs ECS systems.
    class PlayScene : public Scene {
    public:
        PlayScene(Game &game);

        PlayScene(Game &game, std::string level_path);

        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;

        bool is_running() const override;

    private:
        void handle_level_transitions();

        void handle_input();

        void setup_systems();

        void run_update_systems(zia::engine::IEntityManager &registry, float dt);

        void run_render_systems(zia::engine::IEntityManager &registry, const Camera &camera);

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
        InspectorSystem _inspector_system;
        Level _level;
        HUD _hud;
        bool _running = true;
        float _level_transition_delay = 0.0f;
        bool _level_transition_pending = false;

        // Initialize current level path using constants so the default matches the first level.
        std::string _current_level_path = std::string(zia::constants::LEVEL1_PATH);

        // Track the previous state of the ToggleDebug key to perform a rising-edge toggle
        bool _debug_toggle_last_state = false;

        // Ordered update callbacks to keep the ECS steps deterministic.
        std::vector<std::function<void(zia::engine::IEntityManager&, float)>> _update_systems;
        // Render callbacks that rely on the camera context provided each frame.
        std::vector<std::function<void(zia::engine::IEntityManager&, zia::engine::IRenderer&, zia::engine::IAssetManager&, const Camera&)>> _render_systems;

        // Async asset loading future and flag. When true, background decode is running and update() will finalize textures each frame.
        std::future<void> _asset_loading_future;
        bool _assets_loading = false;
    };
} // namespace Zia

