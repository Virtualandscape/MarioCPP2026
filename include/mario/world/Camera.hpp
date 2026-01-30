#pragma once

namespace mario {
    // Follows player, handles parallax bounds.
    class Camera {
    public:
        void set_target(float x, float y);

        void set_bounds(float left, float top, float right, float bottom);

        void update(float dt);
    };
} // namespace mario
