#pragma once

#include "Entity.hpp"

namespace mario {
    // Input handling, jump/run state, power-ups.
    class Player : public Entity {
    public:
        Player();

        void handle_input();

        void set_move_axis(float axis);

        void set_jump_pressed(bool pressed);

        void update(float dt) override;

        void render() override;

    private:
        float move_axis_ = 0.0f;
        bool jump_pressed_ = false;
        float move_speed_ = 220.0f;
        float jump_speed_ = 360.0f;
        float gravity_ = 1200.0f;
    };
} // namespace mario
