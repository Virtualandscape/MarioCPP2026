#pragma once

#include "mario/entities/Player.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/systems/CollisionSystem.hpp"
#include "mario/systems/PhysicsSystem.hpp"
#include "mario/world/Level.hpp"

#include <memory>
#include <vector>

namespace mario {
    class Entity;
    class Game;
    // Common base class for game states like play, menu, and pause
    class GameState {
    public:
        virtual ~GameState() = default;

        virtual void on_enter() = 0;

        virtual void on_exit() = 0;

        virtual void update(float dt) = 0;

        virtual void render() = 0;

        virtual bool is_running() const { return true; }
    };

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
        PhysicsSystem _physics;
        CollisionSystem _collision;
        Level _level;
        std::vector<std::unique_ptr<Entity>> _entities;
        bool _running = true;
        float _level_transition_delay = 0.0f;
        std::string _current_level_path = "assets/levels/level1.json";
    };

    class MenuState : public GameState {
    public:
        MenuState(Game& game);

        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;

        bool is_running() const override;

    private:
        Game& _game;
        std::vector<std::string> _levels;
        int _selected_index = 0;
        bool _running = true;
        bool _up_pressed = false;
        bool _down_pressed = false;
        bool _enter_pressed = false;
    };

    class PauseState : public GameState {
    public:
        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;
    };
} // namespace mario
