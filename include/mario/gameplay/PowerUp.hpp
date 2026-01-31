#pragma once

#include "mario/entities/Entity.hpp"
#include "mario/render/Renderer.hpp"

namespace mario {
    // Mushroom, fire flower, etc.
    class PowerUp : public Entity {
    public:
        void update(float dt) override;
        void render(Renderer& renderer) override;
        virtual void apply_to_player() = 0;
    };

    // Coins, 1‑ups, collectibles
    class Item : public Entity {
    public:
        void update(float dt) override;
        void render(Renderer& renderer) override;
        void collect();
    };

    //  Fireball logic
    class Projectile : public Entity {
    public:
        void update(float dt) override;
        void render(Renderer& renderer) override;
    };
} // namespace mario
