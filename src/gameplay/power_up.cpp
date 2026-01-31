#include "mario/gameplay/PowerUp.hpp"

namespace mario {
    void PowerUp::update(float dt) { (void) dt; }
    void PowerUp::render(Renderer& renderer) {}

    void Item::update(float dt) { (void) dt; }
    void Item::render(Renderer& renderer) {}
    void Item::collect() {
    }

    void Projectile::update(float dt) { (void) dt; }

    void Projectile::render(Renderer& renderer) {
    }
} // namespace mario
