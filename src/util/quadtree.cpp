// Implements the quadtree class for spatial partitioning, used to efficiently query and manage objects in 2D space.
// Provides methods for insertion, clearing, splitting, and querying objects in the quadtree.

#include "../../include/mario/util/quadtree.h"

#include <iostream>

// quadtree constructor: initializes the quadtree with a level and bounding rectangle.
quadtree::quadtree(const int level, const sf::FloatRect& bounds) : level(level), bounds(bounds)
{
    quadTiles.reserve(MAX_OBJECTS);
    nodes.reserve(4);
}

// Clear the quadtree and all its nodes recursively.
auto quadtree::clear() -> void
{
    quadTiles.clear();
    for (auto& node : nodes)
    {
        node.clear();
    }
    nodes.clear();
}

// Split the quadtree into four subnodes.
auto quadtree::split() -> void
{
    const float subWidth = bounds.size.x / 2.0f;
    const float subHeight = bounds.size.y / 2.0f;
    const float x = bounds.position.x;
    const float y = bounds.position.y;

    nodes.emplace_back(level + 1, sf::FloatRect({x + subWidth, y}, {subWidth, subHeight}));
    nodes.emplace_back(level + 1, sf::FloatRect({x, y}, {subWidth, subHeight}));
    nodes.emplace_back(level + 1, sf::FloatRect({x, y + subHeight}, {subWidth, subHeight}));
    nodes.emplace_back(level + 1, sf::FloatRect({x + subWidth, y + subHeight}, {subWidth, subHeight}));
}

// Get the index of the quadrant that the rect might intersect with (0-3) or -1 if it doesn't intersect.
[[nodiscard]]
auto quadtree::getIndex(const sf::FloatRect& rect) const -> int
{
    const float verticalMidpoint = bounds.position.x + bounds.size.x / 2;
    const float horizontalMidpoint = bounds.position.y + bounds.size.y / 2;

    const bool topQuadrant = (rect.position.y < horizontalMidpoint && rect.position.y + rect.size.y < horizontalMidpoint);
    const bool bottomQuadrant = (rect.position.y > horizontalMidpoint);

    if (rect.position.x < verticalMidpoint && rect.position.x + rect.size.x < verticalMidpoint)
    {
        if (topQuadrant) return 1;
        if (bottomQuadrant) return 2;
    }
    else if (rect.position.x > verticalMidpoint)
    {
        if (topQuadrant) return 0;
        if (bottomQuadrant) return 3;
    }

    return -1;
}

// Insert a quadtile into the quadtree node or split the node if it exceeds the maximum number of objects
auto quadtree::insert(const QuadTile& tile) -> void
{
    if (!nodes.empty())
    {
        int index = getIndex(tile.bounds);
        if (index != -1)
        {
            nodes[index].insert(tile);
            return;
        }
    }

    quadTiles.push_back(tile);

    if (quadTiles.size() > MAX_OBJECTS && level < MAX_LEVELS)
    {
        if (nodes.empty()) split();
        size_t i = 0;
        while (i < quadTiles.size())
        {
            int index = getIndex(quadTiles[i].bounds);
            if (index != -1)
            {
                nodes[index].insert(quadTiles[i]);
                quadTiles.erase(quadTiles.begin() + i);
            }
            else
            {
                ++i;
            }
        }
    }
}

// Retrieve all quadtiles that intersect with the specified rect and add them to the returnObjects vector (recursive)
auto quadtree::retrieve(std::vector<QuadTile>& returnObjects, const sf::FloatRect& rect) const -> std::vector<QuadTile>
{
    int index = getIndex(rect);
    if (index != -1 && !nodes.empty())
    {
        nodes[index].retrieve(returnObjects, rect);
    }

    returnObjects.insert(returnObjects.end(), quadTiles.begin(), quadTiles.end());
    return returnObjects;
}

// Print the quadtree structure (for debugging purposes)
auto quadtree::print(int level) const -> void
{
    std::cout << "Level: " << level << " Bounds: " << bounds.position.x << ", " << bounds.position.y << ", " << bounds.size.x << ", " << bounds.size.y << std::endl;
    for (const auto& tile : quadTiles)
    {
        std::cout << "Tile: " << tile.bounds.position.x << ", " << tile.bounds.position.y << ", " << tile.bounds.size.x << ", " << tile.bounds.size.y << std::endl;
    }
    for (const auto& node : nodes)
    {
        node.print(level + 1);
    }
}