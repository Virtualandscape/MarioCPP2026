#pragma once

#include "Entity.hpp"

namespace mario {
    // Input handling, jump/run state, power‑ups.
    class Player : public Entity {
    public:
        void handle_input();

        void update(float dt) override;

        void render() override;
    };
} // namespace mario
