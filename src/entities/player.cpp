#include <cmath>

#include "mario/entities/Player.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    Player::Player() : _jump_count(0) {
        set_size(16.0f, 24.0f);
    }

    void Player::handle_input() {
        set_velocity(_move_axis * _move_speed, vy());

        if (_jump_pressed) {
            if (_jump_count < 2) {
                set_velocity(vx(), -_jump_speed);
                _jump_count++;
            }
            _jump_pressed = false;
        }
    }

    void Player::reset_jump() {
        _jump_count = 0;
    }

    void Player::set_move_axis(float axis) { _move_axis = axis; }

    void Player::set_jump_pressed(bool pressed) {
        if (pressed && !_jump_held) {
            _jump_pressed = true;
        }
        _jump_held = pressed;
    }

    void Player::update(float dt) { (void) dt; }

    bool Player::is_on_ground(const TileMap& map) const {
        const int tile_size = map.tile_size();
        if (tile_size <= 0) {
            return false;
        }

        constexpr float epsilon = 0.1f;
        const float bottom = y() + height();
        const int ty = static_cast<int>(std::floor((bottom + epsilon) / tile_size));
        const int start_tx = static_cast<int>(std::floor(x() / tile_size));
        int end_tx = static_cast<int>(std::floor((x() + width() - epsilon) / tile_size));
        if (end_tx < start_tx) {
            end_tx = start_tx;
        }

        for (int tx = start_tx; tx <= end_tx; ++tx) {
            if (map.is_solid(tx, ty)) {
                return true;
            }
        }

        return false;
    }

    void Player::render() {
    }
} // namespace mario
