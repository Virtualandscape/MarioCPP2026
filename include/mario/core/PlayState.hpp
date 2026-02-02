#pragma once

#include "mario/core/GameState.hpp"
#include "mario/entities/Player.hpp"
#include "mario/systems/CollisionSystem.hpp"
#include "mario/systems/PhysicsSystem.hpp"
#include "mario/systems/PlayerInputSystem.hpp"
#include "mario/systems/PlayerMovementSystem.hpp"
#include "mario/world/Level.hpp"
#include "mario/ui/HUD.hpp"
#include "mario/ecs/Registry.hpp"
#include <memory>
#include <vector>
#include <string>

namespace mario {
    class Entity;
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
        Game& _game;
        Player _player;
        EntityID _player_id;
        PhysicsSystem _physics;
        CollisionSystem _collision;
        PlayerInputSystem _player_input;
        PlayerMovementSystem _player_movement;
        Level _level;
        HUD _hud;
        Registry _registry;
        std::vector<std::unique_ptr<Entity>> _entities;
        bool _running = true;
        float _level_transition_delay = 0.0f;
        std::string _current_level_path = "assets/levels/level1.json";
    };
} // namespace mario
