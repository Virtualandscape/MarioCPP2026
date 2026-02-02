#pragma once

#include "mario/ecs/EntityType.hpp"

namespace mario {
    class Renderer;  // Forward declaration
    

    // Position/velocity, update/render hooks, bounding box.
    class Entity {
    public:
        virtual ~Entity() = default;

        virtual void update(float dt) = 0;

        virtual void render(Renderer& renderer) = 0;

        virtual EntityType type() const { return EntityType::Unknown; }

        void set_position(float x, float y);

        void set_velocity(float vx, float vy);

        void set_size(float width, float height);

        float x() const;
        float y() const;
        float vx() const;
        float vy() const;
        float width() const;
        float height() const;
        void update_position(float dt);

        struct CollisionInfo {
            bool collided = false;
            EntityType other_type = EntityType::Unknown;
        };

        void set_collision_info(const CollisionInfo& info) { _last_collision = info; }
        const CollisionInfo& last_collision() const { return _last_collision; }
        void clear_collision_info() { _last_collision = {}; }

    protected:
        float _x = 0.0f;
        float _y = 0.0f;
        float _vx = 0.0f;
        float _vy = 0.0f;
        float _width = 1.0f;
        float _height = 1.0f;
        CollisionInfo _last_collision;
    };
} // namespace mario
