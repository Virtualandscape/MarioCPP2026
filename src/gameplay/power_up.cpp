#include "mario/gameplay/PowerUp.hpp"

namespace mario {
    void PowerUp::update(float dt) { (void) dt; }
    void PowerUp::render() {}

    void Item::update(float dt) { (void) dt; }
    void Item::render() {}
    void Item::collect() {
    }

    void Projectile::update(float dt) { (void) dt; }

    void Projectile::render() {
    }
} // namespace mario
