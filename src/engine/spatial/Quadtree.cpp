#include "Zia/engine/spatial/Quadtree.hpp"
#include <iostream>
#include <utility>

namespace zia::engine::spatial {

Quadtree::Quadtree(int level, const sf::FloatRect& bounds) : _level(level), _bounds(bounds)
{
    _quadTiles.reserve(MAX_OBJECTS);
    _nodes.reserve(4);
}

void Quadtree::clear()
{
    _quadTiles.clear();
    for (auto& node : _nodes)
    {
        node.clear();
    }
    _nodes.clear();
}

void Quadtree::split()
{
    const float subWidth = _bounds.size.x / 2.0f;
    const float subHeight = _bounds.size.y / 2.0f;
    const float x = _bounds.position.x;
    const float y = _bounds.position.y;

    _nodes.emplace_back(_level + 1, sf::FloatRect({x + subWidth, y}, {subWidth, subHeight}));
    _nodes.emplace_back(_level + 1, sf::FloatRect({x, y}, {subWidth, subHeight}));
    _nodes.emplace_back(_level + 1, sf::FloatRect({x, y + subHeight}, {subWidth, subHeight}));
    _nodes.emplace_back(_level + 1, sf::FloatRect({x + subWidth, y + subHeight}, {subWidth, subHeight}));
}

int Quadtree::getIndex(const sf::FloatRect& rect) const
{
    const float verticalMidpoint = _bounds.position.x + _bounds.size.x / 2;
    const float horizontalMidpoint = _bounds.position.y + _bounds.size.y / 2;

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

void Quadtree::insert(const QuadTile& tile)
{
    if (!_nodes.empty())
    {
        int index = getIndex(tile.bounds);
        if (index != -1)
        {
            _nodes[index].insert(tile);
            return;
        }
    }

    _quadTiles.push_back(tile);

    if (_quadTiles.size() > MAX_OBJECTS && _level < MAX_LEVELS)
    {
        if (_nodes.empty()) split();
        size_t i = 0;
        while (i < _quadTiles.size())
        {
            int index = getIndex(_quadTiles[i].bounds);
            if (index != -1)
            {
                _nodes[index].insert(_quadTiles[i]);
                // Swap-and-pop to avoid O(n) erase in a tight loop.
                std::swap(_quadTiles[i], _quadTiles.back());
                _quadTiles.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }
}

std::vector<QuadTile> Quadtree::retrieve(std::vector<QuadTile>& returnObjects, const sf::FloatRect& rect) const
{
    int index = getIndex(rect);
    if (index != -1 && !_nodes.empty())
    {
        _nodes[index].retrieve(returnObjects, rect);
    }

    returnObjects.insert(returnObjects.end(), _quadTiles.begin(), _quadTiles.end());
    return returnObjects;
}

void Quadtree::print(int level) const
{
    std::cout << "Level: " << level << " Bounds: " << _bounds.position.x << ", " << _bounds.position.y << ", " << _bounds.size.x << ", " << _bounds.size.y << std::endl;
    for (const auto& tile : _quadTiles)
    {
        std::cout << "Tile: " << tile.bounds.position.x << ", " << tile.bounds.position.y << ", " << tile.bounds.size.x << ", " << tile.bounds.size.y << std::endl;
    }
    for (const auto& node : _nodes)
    {
        node.print(level + 1);
    }
}

} // namespace zia::engine::spatial

