#include "Cell.hpp"

Cell::Cell(sf::Vector2f position, sf::Vector2f size, sf::Color outlineColor, sf::Color fillColor, float thickness) {
    shape.setSize(size);
    shape.setPosition(position);
    shape.setOutlineColor(outlineColor);
    shape.setFillColor(fillColor);
    shape.setOutlineThickness(thickness);
}
