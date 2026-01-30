#pragma once

#include "mario/entities/Player.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"

namespace mario {
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
        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;

        bool is_running() const override;

    private:
        InputManager input_;
        Player player_;
        Renderer renderer_;
    };

    class MenuState : public GameState {
    public:
        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;
    };

    class PauseState : public GameState {
    public:
        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;
    };
} // namespace mario
