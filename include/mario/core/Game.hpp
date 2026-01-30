#pragma once

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

    private:
        bool running_ = false;
        std::vector<std::shared_ptr<GameState>> states_;
    };
} // namespace mario
