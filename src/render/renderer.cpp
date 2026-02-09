// cpp
#include "mario/render/Renderer.hpp"
#include "mario/helpers/Constants.hpp"
#include <iostream>

namespace mario {
    Renderer::Renderer()
        : _window(sf::VideoMode({800u, 480u}), "Mario Prototype", sf::Style::Titlebar | sf::Style::Close),
          _camera_scale(mario::constants::TILE_SCALE * mario::constants::CAMERA_SCALE)
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
            float world_w = 0.0f;
            float world_h = 0.0f;
            if (_camera_tiles_w > 0.0f) {
                // Fit _camera_tiles_w tiles across the viewport width
                world_w = _camera_tiles_w * static_cast<float>(mario::constants::TILE_SIZE);
                // Preserve aspect ratio: compute world_h from world_w based on window aspect
                const float aspect = h > 0.0f ? (w / h) : 1.0f;
                world_h = world_w / aspect;
            } else {
                world_w = w * _camera_scale;
                world_h = h * _camera_scale;
            }
            sf::View view;
            view.setSize({world_w, world_h});
            // setCenter expects the center point in world coords: camera x/y are top-left, so add half-size
            view.setCenter({x + world_w * 0.5f, y + world_h * 0.5f});
            _window.setView(view);
        }
    }

    void Renderer::draw_sprite(const sf::Texture* texture, float x, float y, float width, float height, const sf::IntRect& texture_rect) {
        if (!_window.isOpen() || !texture) {
            return;
        }

        sf::Sprite sprite(*texture);
        
        sf::Vector2f source_size;
        if (texture_rect.size.x != 0 && texture_rect.size.y != 0) {
            sprite.setTextureRect(texture_rect);
            source_size = { std::abs(static_cast<float>(texture_rect.size.x)), std::abs(static_cast<float>(texture_rect.size.y)) };
        } else {
            auto tex_size = texture->getSize();
            source_size = { static_cast<float>(tex_size.x), static_cast<float>(tex_size.y) };
        }

        // In SFML 3, a negative size in the texture rect flips the sprite 
        // but keeps the origin at the top-left of the drawing area.
        // Therefore, we don't need manual offsets.
        sprite.setPosition({x, y});

        // Optional scaling if width/height are provided
        if (width > 0.0f && height > 0.0f) {
            sprite.setScale({
                (width / source_size.x),
                (height / source_size.y)
            });
        }

        _window.draw(sprite);
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
        const float w = static_cast<float>(size.x);
        const float h = static_cast<float>(size.y);
        if (_camera_tiles_w > 0.0f) {
            const float world_w = _camera_tiles_w * static_cast<float>(mario::constants::TILE_SIZE);
            const float aspect = h > 0.0f ? (w / h) : 1.0f;
            const float world_h = world_w / aspect;
            return {world_w, world_h};
        }
        // Return viewport size in world coordinates (pixels) so camera and rendering subsystems agree on world extents.
        return {w * _camera_scale,
                h * _camera_scale};
    }
} // namespace mario
