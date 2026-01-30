#pragma once

#include <string_view>

namespace mario {
    // Lives, coins, score, timer
    class HUD {
    public:
        void set_lives(int lives);

        void set_coins(int coins);

        void set_score(int score);

        void set_timer(int seconds);

        void render();
    };

    class Text {
    public:
        void set_string(std::string_view text);

        void render();
    };
} // namespace mario
