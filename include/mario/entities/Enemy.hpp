#pragma once

#include "Entity.hpp"

namespace mario {

    // AI hooks, patrol/move patterns.
    class Enemy : public Entity {
    public:
        void update(float dt) override;

        void render() override;

        virtual void on_stomped() = 0;
    };

    class Goomba : public Enemy {
    public:
        void on_stomped() override;
    };

    class Koopa : public Enemy {
    public:
        void on_stomped() override;
    };
} // namespace mario
