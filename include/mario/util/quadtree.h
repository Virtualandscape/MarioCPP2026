#ifndef QUADTREE_H
#define QUADTREE_H

#include "QuadTile.h"
#include <vector>
#include <SFML/Graphics/Rect.hpp>

namespace sf
{
    class RenderWindow;
}

class quadtree
{
private:
    static constexpr int MAX_OBJECTS = 10;
    static constexpr int MAX_LEVELS = 5;
    int level{};
    std::vector<QuadTile> quadTiles;
    sf::FloatRect bounds;
    std::vector<quadtree> nodes;

public:
    // Constructor
    quadtree(int level, const sf::FloatRect& bounds);
    // Properties
    [[nodiscard]] sf::FloatRect getBounds() const { return bounds; }
    [[nodiscard]] int getIndex(const sf::FloatRect& rect) const;
    // Methods
    void clear();
    void split();
    void insert(const QuadTile& tile);
    std::vector<QuadTile> retrieve(std::vector<QuadTile>& returnObjects, const sf::FloatRect& rect) const;
    void print(int level = 0) const;
};

#endif // QUADTREE_H