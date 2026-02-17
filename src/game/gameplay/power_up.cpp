#include "Zia/game/gameplay/PowerUp.hpp"

namespace zia {
    void PowerUp::update(float dt) { (void) dt; }
    void PowerUp::render(zia::engine::IRenderer& /*renderer*/) {}

    void Item::update(float dt) { (void) dt; }
    void Item::render(zia::engine::IRenderer& /*renderer*/) {}
    void Item::collect() {
    }

    void Projectile::update(float dt) { (void) dt; }

    void Projectile::render(zia::engine::IRenderer& /*renderer*/) {
    }
} // namespace Zia
