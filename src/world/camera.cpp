#include "mario/world/Camera.hpp"

namespace mario {
    void Camera::set_target(float x, float y) {
        (void) x;
        (void) y;
    }

    void Camera::set_bounds(float left, float top, float right, float bottom) {
        (void) left;
        (void) top;
        (void) right;
        (void) bottom;
    }

    void Camera::update(float dt) { (void) dt; }
} // namespace mario
