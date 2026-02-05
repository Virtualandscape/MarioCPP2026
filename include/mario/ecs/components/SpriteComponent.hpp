#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace mario {
    struct SpriteComponent {
        enum class Shape {
            Rectangle,
            Ellipse,
            None
        };

        Shape shape = Shape::Rectangle;
        sf::Color color = sf::Color::White;
        int texture_id = -1;
        sf::Vector2f render_offset = {0.0f, 0.0f};
    };
} // namespace mario
