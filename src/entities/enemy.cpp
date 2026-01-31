#include "mario/entities/Enemy.hpp"
#include "mario/render/Renderer.hpp"

namespace mario {

void Enemy::update(float dt) { (void)dt; }
void Enemy::render(Renderer& renderer) {
    renderer.draw_rect(x(), y(), width(), height());
}

Goomba::Goomba() {
    set_size(16.0f, 16.0f);
}

void Goomba::render(Renderer& renderer) {
    renderer.draw_rect(x(), y(), width(), height(), sf::Color::Black);
}

void Goomba::on_stomped() {}

Koopa::Koopa() {
    set_size(16.0f, 16.0f);
}

void Koopa::render(Renderer& renderer) {
    renderer.draw_rect(x(), y(), width(), height(), sf::Color::Red);
}

void Koopa::on_stomped() {}

} // namespace mario
