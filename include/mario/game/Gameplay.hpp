#pragma once

#include "mario/render/Renderer.hpp"

namespace mario {
    // Collectible power-ups (Mushroom, fire flower, etc).
    class PowerUp {
    public:
        virtual ~PowerUp() = default;
        virtual void update(float dt) = 0;
        virtual void render(Renderer& renderer) = 0;
        virtual void apply_to_player() = 0;
    };

    // Generic items (Coins, 1‑ups).
    class Item {
    public:
        void update(float dt);
        void render(Renderer& renderer);
        void collect();
    };

    // Moveable projectiles (Fireballs).
    class Projectile {
    public:
        void update(float dt);
        void render(Renderer& renderer);
    };
} // namespace mario

