#pragma once

#include "mario/core/GameState.hpp"

namespace mario {
    class PauseState : public GameState {
    public:
        void on_enter() override;
        void on_exit() override;
        void update(float dt) override;
        void render() override;
    };
} // namespace mario
