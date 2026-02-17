#include "Zia/engine/render/Sprite.hpp"

namespace zia {

void Sprite::set_texture(int texture_id) { (void)texture_id; }
void Sprite::set_origin(float x, float y) { (void)x; (void)y; }

void Animation::update(float dt) { (void)dt; }
void Animation::play(int clip_id) { (void)clip_id; }

} // namespace Zia
