#include "mario/entities/Player.hpp"

namespace mario {

Player::Player()
{
    set_size(16.0f, 24.0f);
}

void Player::handle_input()
{
    set_velocity(_move_axis * _move_speed, vy());

    if (_jump_pressed) {
        set_velocity(vx(), -_jump_speed);
        _jump_pressed = false;
    }
}

void Player::set_move_axis(float axis) { _move_axis = axis; }

void Player::set_jump_pressed(bool pressed) { _jump_pressed = pressed; }

void Player::update(float dt)
{
    set_velocity(vx(), vy() + _gravity * dt);
    integrate(dt);
}

void Player::render() {}

} // namespace mario
