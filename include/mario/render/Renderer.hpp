#pragma once

#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>

#include "mario/util/constants.hpp"

namespace mario {
    // Draw calls, sprites, layers, parallax
    class Renderer {
    public:
        Renderer();

        void begin_frame();

        void end_frame();

        void set_camera(float x, float y);

        void draw_sprite(int sprite_id, float x, float y);

        void draw_rect(float x, float y, float width, float height, sf::Color color = constants::TILE_COLOR);

        void draw_text(const std::string& text, float x, float y, unsigned int size = 20, sf::Color color = sf::Color::White);

        sf::RenderWindow& window() { return _window; }

        void draw_ellipse(float x, float y, float width, float height, sf::Color color = sf::Color(240, 240, 240));

        bool is_open() const;

        sf::Vector2f viewport_size() const;

    private:
        sf::RenderWindow _window;
        sf::Font _font;
        sf::Color _clear_color = sf::Color(30, 30, 36);
        float _camera_x = 0.0f;
        float _camera_y = 0.0f;
    };
} // namespace mario
