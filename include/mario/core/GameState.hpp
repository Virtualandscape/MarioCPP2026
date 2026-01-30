#pragma once

namespace mario {
    // Common base class for game states like play, menu, and pause
    class GameState {
    public:
        virtual ~GameState() = default;

        virtual void on_enter() = 0;

        virtual void on_exit() = 0;

        virtual void update(float dt) = 0;

        virtual void render() = 0;
    };

    class PlayState : public GameState {
    public:
        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;
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
