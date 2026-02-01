#pragma once

#include "mario/render/Renderer.hpp"
#include "mario/input/InputManager.hpp"
#include <memory>
#include <vector>

namespace mario {
    class GameState;

    // Owns the main loop, initialization/shutdown, high‑level state
    class Game {
    public:
        void initialize();

        void shutdown();

        void run();

        void push_state(std::shared_ptr<GameState> state);

        void pop_state();

        std::shared_ptr<GameState> current_state();

        Renderer& renderer() { return _renderer; }
        InputManager& input() { return _input; }

    private:
        bool _running = false;
        std::vector<std::shared_ptr<GameState>> _states;
        Renderer _renderer;
        InputManager _input;
    };
} // namespace mario
