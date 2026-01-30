#pragma once

namespace mario {
    // Follows player, handles parallax bounds.
    class Camera {
    public:
        void set_target(float x, float y);

        void set_bounds(float left, float top, float right, float bottom);

        void set_viewport(float width, float height);

        void update(float dt);

        float x() const;

        float y() const;

    private:
        float _x = 0.0f;
        float _y = 0.0f;
        float _target_x = 0.0f;
        float _target_y = 0.0f;
        float _left = 0.0f;
        float _top = 0.0f;
        float _right = 0.0f;
        float _bottom = 0.0f;
        float _viewport_w = 0.0f;
        float _viewport_h = 0.0f;
        float _follow_speed = 8.0f;
    };
} // namespace mario
