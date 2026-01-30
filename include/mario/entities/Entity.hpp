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

        float _x = 0.0f;
        float _y = 0.0f;
        float _vx = 0.0f;
        float _vy = 0.0f;
        float _width = 1.0f;
        float _height = 1.0f;
    };
} // namespace mario
