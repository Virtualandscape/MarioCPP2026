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
    };
} // namespace mario
