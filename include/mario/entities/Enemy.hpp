#pragma once

#include "Entity.hpp"

namespace mario {

    // AI hooks, patrol/move patterns.
    class Enemy : public Entity {
    public:
        void update(float dt) override;

        void render(Renderer& renderer) override;

        virtual void on_stomped() = 0;
    };

    class Goomba : public Enemy {
    public:
        Goomba();
        void render(Renderer& renderer) override;
        void on_stomped() override;
        EntityTypeComponent type() const override { return EntityTypeComponent::Goomba; }
    };

    class Koopa : public Enemy {
    public:
        Koopa();
        void render(Renderer& renderer) override;
        void on_stomped() override;
        EntityTypeComponent type() const override { return EntityTypeComponent::Koopa; }
    };
} // namespace mario
