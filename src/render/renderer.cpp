// cpp
#include "mario/render/Renderer.hpp"
#include <iostream>

namespace mario {
    Renderer::Renderer()
        : _window(sf::VideoMode({800u, 480u}), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close)
    {
        _window.setVerticalSyncEnabled(true);

        const std::vector<std::string> font_paths = {
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "../../assets/fonts/arial.ttf"
        };

        bool loaded = false;
        for (const auto& path : font_paths) {
            if (_font.openFromFile(path)) {
                loaded = true;
                break;
            }
        }

        if (!loaded) {
            std::cerr << "Failed to load font from any of the standard paths." << std::endl;
        }
    }

    void Renderer::begin_frame() {
        if (!_window.isOpen()) {
            return;
        }

        while (const auto event = _window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                _window.close();
            }
        }

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

        // Update the SFML view so drawing to the RenderWindow uses world coordinates
        if (_window.isOpen()) {
            const auto size = _window.getSize();
            const float w = static_cast<float>(size.x);
            const float h = static_cast<float>(size.y);
            sf::View view;
            view.setSize({w, h});
            // setCenter expects the center point: camera x/y are top-left, so add half-size
            view.setCenter({x + w * 0.5f, y + h * 0.5f});
            _window.setView(view);
        }
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
        // Positions are world-space; the current SFML view (set in set_camera) handles camera transform.
        shape.setPosition({x, y});
        shape.setFillColor(color);
        _window.draw(shape);
    }

    void Renderer::draw_text(const std::string& text, float x, float y, unsigned int size, sf::Color color) {
        if (!_window.isOpen()) {
            return;
        }

        // Draw text in screen/UI space: temporarily switch to default view
        sf::RenderWindow& window = _window;
        auto old_view = window.getView();
        window.setView(window.getDefaultView());

        // Construct sf::Text using the overload that accepts sf::Font first (compatible with some SFML versions)
        sf::Text sf_text(_font);
        sf_text.setString(text);
        sf_text.setCharacterSize(size);
        sf_text.setFillColor(color);
        sf_text.setPosition({x, y}); // UI-space
        window.draw(sf_text);

        window.setView(old_view);
    }

    void Renderer::draw_ellipse(float x, float y, float width, float height, sf::Color color) {
        if (!_window.isOpen()) {
            return;
        }

        sf::CircleShape shape(0.5f);
        shape.setScale({width, height});
        // Positions are world-space; rely on SFML view transform instead of manual offset
        shape.setPosition({x, y});
        shape.setFillColor(color);
        _window.draw(shape);
    }

    // Draw an unfilled outline rectangle in world-space to visualize bounding boxes.
    void Renderer::draw_bbox(float x, float y, float width, float height, sf::Color color, float thickness) {
        if (!_window.isOpen()) {
            return;
        }
        sf::RectangleShape outline({width, height});
        // Transparent fill and colored outline
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(color);
        outline.setOutlineThickness(thickness);
        outline.setPosition({x, y});
        _window.draw(outline);
    }

    void Renderer::toggle_debug_bboxes() {
        _debug_bboxes = !_debug_bboxes;
    }

    bool Renderer::is_debug_bboxes_enabled() const { return _debug_bboxes; }

    bool Renderer::is_open() const { return _window.isOpen(); }

    sf::Vector2f Renderer::viewport_size() const {
        const auto size = _window.getSize();
        return {static_cast<float>(size.x), static_cast<float>(size.y)};
    }
} // namespace mario
