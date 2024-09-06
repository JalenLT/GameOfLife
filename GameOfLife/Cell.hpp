#ifndef CELL_HPP
#define CELL_HPP

#include <SFML/Graphics.hpp>

class Cell {
public:
    sf::RectangleShape shape;
    bool isAlive = false;

    Cell(sf::Vector2f position, sf::Vector2f size, sf::Color outlineColor = sf::Color(50, 50, 50), sf::Color fillColor = sf::Color(0, 0, 0), float thickness = 3.0f);
};

#endif
