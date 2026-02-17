#pragma once

namespace zia {
    //  frame sets, timing, flip
    class Sprite {
    public:
        void set_texture(int texture_id);

        void set_origin(float x, float y);
    };

    class Animation {
    public:
        void update(float dt);

        void play(int clip_id);
    };
} // namespace Zia
