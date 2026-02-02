#pragma once

#include <SFML/Graphics/Color.hpp>

namespace mario {
    struct Sprite {
        enum class Shape {
            Rectangle,
            Ellipse
        };

        Shape shape = Shape::Rectangle;
        sf::Color color = sf::Color::White;
    };
} // namespace mario
