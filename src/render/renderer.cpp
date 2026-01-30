#include "mario/render/Renderer.hpp"

namespace mario {

Renderer::Renderer()
#if SFML_VERSION_MAJOR >= 3
    : window_(sf::VideoMode({800u, 480u}), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
#else
    : window_(sf::VideoMode(800, 480), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
#endif
{
    window_.setVerticalSyncEnabled(true);
}

void Renderer::begin_frame()
{
    if (!window_.isOpen()) {
        return;
    }

#if SFML_VERSION_MAJOR >= 3
    while (const auto event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
        }
    }
#else
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }
    }
#endif

    window_.clear(clear_color_);
}

void Renderer::end_frame()
{
    if (!window_.isOpen()) {
        return;
    }

    window_.display();
}

void Renderer::draw_sprite(int sprite_id, float x, float y)
{
    (void)sprite_id;
    (void)x;
    (void)y;
}

void Renderer::draw_rect(float x, float y, float width, float height)
{
    if (!window_.isOpen()) {
        return;
    }

    sf::RectangleShape shape({width, height});
    shape.setPosition({x, y});
    shape.setFillColor(sf::Color(240, 240, 240));
    window_.draw(shape);
}

void Renderer::draw_ellipse(float x, float y, float width, float height)
{
    if (!window_.isOpen()) {
        return;
    }

    sf::CircleShape shape(0.5f);
    shape.setScale({width, height});
    shape.setPosition({x, y});
    shape.setFillColor(sf::Color(120, 200, 120));
    window_.draw(shape);
}

bool Renderer::is_open() const { return window_.isOpen(); }

} // namespace mario
