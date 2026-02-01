#include "mario/render/Renderer.hpp"
#include <iostream>

namespace mario {
    Renderer::Renderer()
#if SFML_VERSION_MAJOR >= 3
        : _window(sf::VideoMode({800u, 480u}), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
#else
        : _window(sf::VideoMode(800, 480), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
#endif
    {
        _window.setVerticalSyncEnabled(true);
        
        const std::vector<std::string> font_paths = {
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "../../assets/fonts/arial.ttf"
        };
        
        bool loaded = false;
        for (const auto& path : font_paths) {
#if SFML_VERSION_MAJOR >= 3
            if (_font.openFromFile(path)) {
                loaded = true;
                break;
            }
#else
            if (_font.loadFromFile(path)) {
                loaded = true;
                break;
            }
#endif
        }
        
        if (!loaded) {
            std::cerr << "Failed to load font from any of the standard paths." << std::endl;
        }
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

    void Renderer::draw_text(const std::string& text, float x, float y, unsigned int size, sf::Color color) {
        if (!_window.isOpen()) {
            return;
        }

#if SFML_VERSION_MAJOR >= 3
        sf::Text sf_text(_font, text, size);
#else
        sf::Text sf_text;
        sf_text.setFont(_font);
        sf_text.setString(text);
        sf_text.setCharacterSize(size);
#endif
        sf_text.setFillColor(color);
        sf_text.setPosition({x, y}); // Text is usually UI-space
        _window.draw(sf_text);
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
