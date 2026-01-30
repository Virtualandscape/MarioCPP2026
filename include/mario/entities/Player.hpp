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
        float _move_axis = 0.0f;
        bool _jump_pressed = false;
        float _move_speed = 220.0f;
        float _jump_speed = 360.0f;
    };
} // namespace mario
