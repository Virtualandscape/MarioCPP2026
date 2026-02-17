// ...existing code...
#ifndef ZIA_ENGINE_SPATIAL_QUADTILE_H
#define ZIA_ENGINE_SPATIAL_QUADTILE_H

#include <string>
#include <cstdint>
#include <utility>
#include <SFML/Graphics/Rect.hpp>

// QuadTile: small POD representing a tile or AABB stored in the quadtree.
struct QuadTile
{
    sf::FloatRect bounds;
    std::string type;
    std::uint32_t id = 0;

    // Construct from bounds + type.
    QuadTile(const sf::FloatRect& bounds, std::string type)
        : bounds(bounds), type(std::move(type))
    {
    }

    // Construct from bounds + id.
    QuadTile(const sf::FloatRect& bounds, std::uint32_t id)
        : bounds(bounds), id(id)
    {
    }
};

#endif // ZIA_ENGINE_SPATIAL_QUADTILE_H

