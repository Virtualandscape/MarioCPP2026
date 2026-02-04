// Implements the HUD (Heads-Up Display) and Text classes for rendering game information on the screen.
// HUD displays lives, coins, score, timer, and level name. Text handles rendering individual text elements.

#include "mario/ui/HUD.hpp"
#include "mario/render/Renderer.hpp"

namespace mario {
    // HUD constructor initializes with a reference to the renderer.
    HUD::HUD(Renderer& renderer) : _renderer(renderer) {}

    // Setters for HUD information.
    void HUD::set_lives(int lives) { _lives = lives; }
    void HUD::set_coins(int coins) { _coins = coins; }
    void HUD::set_score(int score) { _score = score; }
    void HUD::set_timer(int seconds) { _timer = seconds; }
    void HUD::set_level_name(std::string_view name) { _level_name = name; }

    // Renders the HUD, currently only displays the level name.
    void HUD::render() {
        if (!_level_name.empty()) {
            _renderer.draw_text(_level_name, 10, 10, 24, sf::Color::White);
        }
        // Future: render lives, coins, etc.
    }

    // Text class constructor initializes with a reference to the renderer.
    Text::Text(Renderer& renderer) : _renderer(renderer) {}

    // Setters for text properties.
    void Text::set_string(std::string_view text) { _text = text; }
    void Text::set_position(float x, float y) { _x = x; _y = y; }
    void Text::set_size(unsigned int size) { _size = size; }
    void Text::set_color(sf::Color color) { _color = color; }

    // Renders the text using the renderer.
    void Text::render() {
        _renderer.draw_text(_text, _x, _y, _size, _color);
    }
} // namespace mario
