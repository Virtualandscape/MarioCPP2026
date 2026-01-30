#include "mario/entities/Player.hpp"

namespace mario {

Player::Player()
{
    set_size(16.0f, 24.0f);
}

void Player::handle_input()
{
    set_velocity(move_axis_ * move_speed_, vy());

    if (jump_pressed_) {
        set_velocity(vx(), -jump_speed_);
        jump_pressed_ = false;
    }
}

void Player::set_move_axis(float axis) { move_axis_ = axis; }

void Player::set_jump_pressed(bool pressed) { jump_pressed_ = pressed; }

void Player::update(float dt)
{
    set_velocity(vx(), vy() + gravity_ * dt);
    integrate(dt);
}

void Player::render() {}

} // namespace mario
