#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
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
        // Offset applied when rendering the sprite relative to entity PositionComponent
        sf::Vector2f render_offset = {0.0f, 0.0f};
        // Optional render size: if both components > 0, use this for drawing instead of SizeComponent
        sf::Vector2f render_size = {0.0f, 0.0f};
        // Optional texture sub-rectangle (for animation frames)
        sf::IntRect texture_rect;
    };
} // namespace mario
