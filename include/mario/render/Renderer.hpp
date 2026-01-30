#pragma once

namespace mario {
    // Draw calls, sprites, layers, parallax
    class Renderer {
    public:
        void begin_frame();

        void end_frame();

        void draw_sprite(int sprite_id, float x, float y);
    };
} // namespace mario
