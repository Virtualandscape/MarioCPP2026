#pragma once

#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include "Zia/engine/spatial/QuadTile.h"

namespace zia::engine::spatial {

// Quadtree: spatial index for broadphase collision / queries.
class Quadtree {
private:
    static constexpr int MAX_OBJECTS = 10;
    static constexpr int MAX_LEVELS = 5;
    int _level{};
    std::vector<QuadTile> _quadTiles;
    sf::FloatRect _bounds;
    std::vector<Quadtree> _nodes;

public:
    // Construct a quadtree node at a level with bounds.
    Quadtree(int level, const sf::FloatRect& bounds);

    [[nodiscard]] sf::FloatRect getBounds() const { return _bounds; }
    [[nodiscard]] int getIndex(const sf::FloatRect& rect) const;

    void clear();
    void split();
    void insert(const QuadTile& tile);
    std::vector<QuadTile> retrieve(std::vector<QuadTile>& returnObjects, const sf::FloatRect& rect) const;
    void print(int level = 0) const;
};

} // namespace zia::engine::spatial

