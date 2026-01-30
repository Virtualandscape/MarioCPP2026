#include "mario/systems/CollisionSystem.hpp"

namespace mario {

void CollisionSystem::update() {}
void Collider::set_solid(bool solid) { (void)solid; }
void Hitbox::set_size(float w, float h) { (void)w; (void)h; }

} // namespace mario
