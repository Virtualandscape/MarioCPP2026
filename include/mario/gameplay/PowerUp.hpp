#pragma once

#include "mario/engine/IRenderer.hpp"

namespace mario {
    // Mushroom, fire flower, etc.
    class PowerUp {
    public:
        void update(float dt) ;
        void render(mario::engine::IRenderer& renderer) ;
        virtual void apply_to_player() = 0;
    };

    // Coins, 1‑ups, collectibles
    class Item {
    public:
        void update(float dt) ;
        void render(mario::engine::IRenderer& renderer);
        void collect();
    };

    //  Fireball logic
    class Projectile {
    public:
        void update(float dt);
        void render(mario::engine::IRenderer& renderer);
    };
} // namespace mario
