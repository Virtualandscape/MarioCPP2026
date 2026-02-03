#ifndef QUADTILE_H
#define QUADTILE_H

#include <string>
#include <SFML/Graphics/Rect.hpp>

struct QuadTile
{
    sf::FloatRect bounds;
    std::string type;

    QuadTile(const sf::FloatRect& bounds, const std::string& type)
        : bounds(bounds), type(type)
    {
    }
};

#endif // QUADTILE_H