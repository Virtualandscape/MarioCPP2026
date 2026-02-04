#pragma once

#include "mario/render/Renderer.hpp"

namespace mario {
    // Mushroom, fire flower, etc.
    class PowerUp {
    public:
        void update(float dt) ;
        void render(Renderer& renderer) ;
        virtual void apply_to_player() = 0;
    };

    // Coins, 1‑ups, collectibles
    class Item {
    public:
        void update(float dt) ;
        void render(Renderer& renderer);
        void collect();
    };

    //  Fireball logic
    class Projectile {
    public:
        void update(float dt);
        void render(Renderer& renderer);
    };
} // namespace mario
