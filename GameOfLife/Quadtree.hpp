#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "Cell.hpp"

class Quadtree {
public:
    sf::FloatRect bounds;
    Quadtree* nodes[4];
    std::vector<int> objectIndices;
    int level;
    int maxObjects;
    int maxLevels;

    Quadtree(sf::FloatRect bounds, int level = 0, int maxObjects = 4, int maxLevels = 5);
    ~Quadtree();

    void clear();
    void split();
    int getIndex(const sf::FloatRect& rect, const std::vector<Cell>& cells);
    void insert(int rectIndex, std::vector<Cell>& cells);
    std::vector<int> retrieve(const sf::FloatRect& rect, const std::vector<Cell>& cells);
};

#endif
