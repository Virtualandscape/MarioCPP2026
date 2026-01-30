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

        float max_x = _right - _viewport_w;
        float max_y = _bottom - _viewport_h;
        if (max_x < _left) {
            max_x = _left;
        }
        if (max_y < _top) {
            max_y = _top;
        }

        const float clamped_x = std::clamp(desired_x, _left, max_x);
        const float clamped_y = std::clamp(desired_y, _top, max_y);

        const float blend = 1.0f - std::exp(-_follow_speed * dt);
        _x += (clamped_x - _x) * blend;
        _y += (clamped_y - _y) * blend;
    }

    float Camera::x() const { return _x; }

    float Camera::y() const { return _y; }
} // namespace mario
