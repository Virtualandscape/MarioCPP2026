#include "mario/entities/Enemy.hpp"
#include "mario/render/Renderer.hpp"

namespace mario {

void Enemy::update(float dt) { (void)dt; }
    void Enemy::render(Renderer& renderer) {
    renderer.draw_rect(x(), y(), width(), height());
}
void Goomba::on_stomped() {}
void Koopa::on_stomped() {}

} // namespace mario
