#pragma once

#include "mario/core/GameState.hpp"
#include "mario/systems/CollisionSystem.hpp"
#include "mario/systems/PhysicsSystem.hpp"
#include "mario/systems/PlayerInputSystem.hpp"
#include "mario/systems/PlayerMovementSystem.hpp"
#include "mario/systems/EnemySystem.hpp"
#include "mario/systems/LevelSystem.hpp"
#include "mario/world/Level.hpp"
#include "mario/world/TileMap.hpp"
#include "mario/ui/HUD.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include <memory>
#include <string>

namespace mario {
    class Game;

    class PlayState : public GameState {
    public:
        PlayState(Game& game);
        PlayState(Game& game, std::string level_path);

        void on_enter() override;
        void on_exit() override;
        void update(float dt) override;
        void render() override;
        bool is_running() const override;

    private:
        void update_camera();
        void handle_level_transitions();
        void handle_input();

        Game& _game;
        EntityID _player_id;
        PhysicsSystem _physics;
        CollisionSystem _collision;
        PlayerInputSystem _player_input;
        PlayerMovementSystem _player_movement;
        EnemySystem _enemy_system;
        Level _level;
        HUD _hud;
        EntityManager _registry;
        bool _running = true;
        float _level_transition_delay = 0.0f;
        std::string _current_level_path = "assets/levels/level1.json";
    };
} // namespace mario
