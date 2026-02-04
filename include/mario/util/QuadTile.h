#ifndef QUADTILE_H
#define QUADTILE_H

#include <string>
#include <cstdint>
#include <utility>
#include <SFML/Graphics/Rect.hpp>

struct QuadTile
{
    sf::FloatRect bounds;
    std::string type;
    std::uint32_t id = 0;

    QuadTile(const sf::FloatRect& bounds, std::string type)
        : bounds(bounds), type(std::move(type))
    {
    }

    QuadTile(const sf::FloatRect& bounds, std::uint32_t id)
        : bounds(bounds), id(id)
    {
    }
};

#endif // QUADTILE_H

