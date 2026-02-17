#pragma once

namespace zia {
    // Music, SFX, volume, channels
    class AudioManager {
    public:
        void play_music(int track_id);

        void play_sfx(int sfx_id);

        void set_volume(float volume);
    };
} // namespace Zia
