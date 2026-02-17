#pragma once

namespace zia {
    // Follows player, handles parallax bounds.
    class Camera {
    public:
        void set_target(float x, float y);

        void set_bounds(float left, float top, float right, float bottom);

        void set_viewport(float width, float height);

        void update(float dt);

        [[nodiscard]] float x() const noexcept;

        [[nodiscard]] float y() const noexcept;

        // Move the camera partially towards the current target.
        // fraction is in [0,1]: 0 = no change, 1 = snap to target.
        // If ignore_bounds is true, the camera will move towards the unclamped desired center
        // (useful for an initial center at level start even if near map edges).
        void center_on_target_fraction(float fraction, bool ignore_bounds = false);

        // Set the camera position directly (overrides internal _x/_y).
        void set_position(float x, float y) noexcept;

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
} // namespace Zia
