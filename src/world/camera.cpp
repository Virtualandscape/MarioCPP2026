#include "mario/world/Camera.hpp"

#include <algorithm>
#include <cmath>

namespace mario {
    void Camera::set_target(float x, float y) {
        _target_x = x;
        _target_y = y;
    }

    void Camera::set_bounds(float left, float top, float right, float bottom) {
        _left = left;
        _top = top;
        _right = right;
        _bottom = bottom;
    }

    void Camera::set_viewport(float width, float height) {
        _viewport_w = width;
        _viewport_h = height;
    }

    void Camera::update(float dt) {
        const float desired_x = _target_x - (_viewport_w * 0.5f);
        const float desired_y = _target_y - (_viewport_h * 0.5f);

        const float blend = 1.0f - std::exp(-5.0f * dt);
        _x += (desired_x - _x) * blend;
        _y += (desired_y - _y) * blend;
    }

    void Camera::center_on_target_fraction(float fraction, bool ignore_bounds) {
        // Clamp fraction to [0,1]
        if (fraction <= 0.0f) return;
        if (fraction > 1.0f) fraction = 1.0f;

        const float desired_x = _target_x - (_viewport_w * 0.5f);
        const float desired_y = _target_y - (_viewport_h * 0.5f);

        float max_x = _right - _viewport_w;
        float max_y = _bottom - _viewport_h;
        if (max_x < _left) {
            max_x = _left;
        }
        if (max_y < _top) {
            max_y = _top;
        }

        float target_x = desired_x;
        float target_y = desired_y;
        if (!ignore_bounds) {
            target_x = std::clamp(desired_x, _left, max_x);
            target_y = std::clamp(desired_y, _top, max_y);
        }

        _x += (target_x - _x) * fraction;
        _y += (target_y - _y) * fraction;
    }

    void Camera::set_position(float x, float y) noexcept {
        _x = x;
        _y = y;
    }

    [[nodiscard]] float Camera::x() const noexcept { return _x; }

    [[nodiscard]] float Camera::y() const noexcept { return _y; }
} // namespace mario
