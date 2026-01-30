#include "mario/render/Renderer.hpp"

namespace mario {

void Renderer::begin_frame() {}
void Renderer::end_frame() {}
void Renderer::draw_sprite(int sprite_id, float x, float y)
{
    (void)sprite_id;
    (void)x;
    (void)y;
}

} // namespace mario
