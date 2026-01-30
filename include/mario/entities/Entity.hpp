#pragma once

namespace mario {
    // Position/velocity, update/render hooks, bounding box.
    class Entity {
    public:
        virtual ~Entity() = default;

        virtual void update(float dt) = 0;

        virtual void render() = 0;

        void set_position(float x, float y);

        void set_velocity(float vx, float vy);

        void set_size(float width, float height);

        float x() const;
        float y() const;
        float vx() const;
        float vy() const;
        float width() const;
        float height() const;

    protected:
        void integrate(float dt);

        float x_ = 0.0f;
        float y_ = 0.0f;
        float vx_ = 0.0f;
        float vy_ = 0.0f;
        float width_ = 1.0f;
        float height_ = 1.0f;
    };
} // namespace mario
