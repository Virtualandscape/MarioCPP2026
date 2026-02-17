#include "mario/game/gameplay/PowerUp.hpp"

namespace mario {
    void PowerUp::update(float dt) { (void) dt; }
    void PowerUp::render(mario::engine::IRenderer& /*renderer*/) {}

    void Item::update(float dt) { (void) dt; }
    void Item::render(mario::engine::IRenderer& /*renderer*/) {}
    void Item::collect() {
    }

    void Projectile::update(float dt) { (void) dt; }

    void Projectile::render(mario::engine::IRenderer& /*renderer*/) {
    }
} // namespace mario
