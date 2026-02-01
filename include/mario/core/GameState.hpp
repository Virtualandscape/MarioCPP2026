#pragma once

#include <memory>

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
} // namespace mario
