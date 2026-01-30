#pragma once

namespace mario {
    // Mushroom, fire flower, etc.
    class PowerUp {
    public:
        virtual ~PowerUp() = default;

        virtual void apply_to_player() = 0;
    };

    // Coins, 1‑ups, collectibles
    class Item {
    public:
        void collect();
    };

    //  Fireball logic
    class Projectile {
    public:
        void update(float dt);

        void render();
    };
} // namespace mario
