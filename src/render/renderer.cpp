#include "mario/render/Renderer.hpp"

namespace mario {
    Renderer::Renderer()
#if SFML_VERSION_MAJOR >= 3
        : _window(sf::VideoMode({800u, 480u}), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
#else
        : _window(sf::VideoMode(800, 480), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
#endif
    {
        _window.setVerticalSyncEnabled(true);
    }

    void Renderer::begin_frame() {
        if (!_window.isOpen()) {
            return;
        }

#if SFML_VERSION_MAJOR >= 3
        while (const auto event = _window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                _window.close();
            }
        }
#else
        sf::Event event;
        while (_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                _window.close();
            }
        }
#endif

        _window.clear(_clear_color);
    }

    void Renderer::end_frame() {
        if (!_window.isOpen()) {
            return;
        }

        _window.display();
    }

    void Renderer::set_camera(float x, float y) {
        _camera_x = x;
        _camera_y = y;
    }

    void Renderer::draw_sprite(int sprite_id, float x, float y) {
        (void) sprite_id;
        (void) x;
        (void) y;
    }

    void Renderer::draw_rect(float x, float y, float width, float height, sf::Color color) {
        if (!_window.isOpen()) {
            return;
        }

        sf::RectangleShape shape({width, height});
        shape.setPosition({x - _camera_x, y - _camera_y});
        shape.setFillColor(color);
        _window.draw(shape);
    }

    void Renderer::draw_ellipse(float x, float y, float width, float height) {
        if (!_window.isOpen()) {
            return;
        }

        sf::CircleShape shape(0.5f);
        shape.setScale({width, height});
        shape.setPosition({x - _camera_x, y - _camera_y});
        shape.setFillColor(sf::Color(120, 200, 120));
        _window.draw(shape);
    }

    bool Renderer::is_open() const { return _window.isOpen(); }

    sf::Vector2f Renderer::viewport_size() const {
        const auto size = _window.getSize();
        return {static_cast<float>(size.x), static_cast<float>(size.y)};
    }
} // namespace mario
