#pragma once

#include <string_view>
#include <string>
#include <SFML/Graphics/Color.hpp>
#include "mario/engine/IRenderer.hpp"

namespace mario {
    class HUD {
    public:
        HUD(mario::engine::IRenderer& renderer);

        void set_lives(int lives);

        void set_coins(int coins);

        void set_score(int score);

        void set_timer(int seconds);

        void set_level_name(std::string_view name);

        void render();

    private:
        mario::engine::IRenderer& _renderer;
        std::string _level_name;
        int _lives = 0;
        int _coins = 0;
        int _score = 0;
        int _timer = 0;
    };

    class Text {
    public:
        Text(mario::engine::IRenderer& renderer);

        void set_string(std::string_view text);

        void set_position(float x, float y);

        void set_size(unsigned int size);

        void set_color(sf::Color color);

        void render();

    private:
        mario::engine::IRenderer& _renderer;
        std::string _text;
        float _x = 0;
        float _y = 0;
        unsigned int _size = 20;
        sf::Color _color = sf::Color::White;
    };
} // namespace mario
