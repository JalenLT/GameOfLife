#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

const int windowWidth{ 500 };
const int windowHeight{ 500 };
const float cellSize{ 20 };
const int cellThickness{ 3 };

class Cell {
public:
    sf::RectangleShape shape;
    bool isAlive = false;

    Cell(sf::Vector2f position, sf::Vector2f size, sf::Color outlineColor = sf::Color(50, 50, 50), sf::Color fillColor = sf::Color(0, 0, 0), float thickness = 3.0f) {
        shape.setSize(size);
        shape.setPosition(position);
        shape.setOutlineColor(outlineColor);
        shape.setFillColor(fillColor);
        shape.setOutlineThickness(thickness);
    }
};

class Quadtree {
public:
    sf::FloatRect bounds;
    Quadtree* nodes[4];
    std::vector<int> objectIndices;
    int level;
    int maxObjects;
    int maxLevels;

    Quadtree(sf::FloatRect bounds, int level = 0, int maxObjects = 4, int maxLevels = 5)
        : bounds(bounds), level(level), maxObjects(maxObjects), maxLevels(maxLevels) {
        for (int i = 0; i < 4; i++) {
            nodes[i] = nullptr;
        }
    }

    ~Quadtree() {
        for (int i = 0; i < 4; i++) {
            delete nodes[i];
        }
    }

    void clear() {
        objectIndices.clear();
        if (nodes[0] != nullptr) {
            for (int i = 0; i < 4; i++) {
                nodes[i]->clear();
            }
        }
    }

    void split() {
        float subWidth = bounds.width / 2;
        float subHeight = bounds.height / 2;

        nodes[0] = new Quadtree(sf::FloatRect(bounds.left, bounds.top, subWidth, subHeight), level + 1, maxObjects, maxLevels); // Top Left
        nodes[1] = new Quadtree(sf::FloatRect(bounds.left + subWidth, bounds.top, subWidth, subHeight), level + 1, maxObjects, maxLevels); // Top Right
        nodes[2] = new Quadtree(sf::FloatRect(bounds.left, bounds.top + subHeight, subWidth, subHeight), level + 1, maxObjects, maxLevels); // Bottom Left
        nodes[3] = new Quadtree(sf::FloatRect(bounds.left + subWidth, bounds.top + subHeight, subWidth, subHeight), level + 1, maxObjects, maxLevels); // Bottom Right
    }

    int getIndex(const sf::FloatRect& rect, const std::vector<Cell>& cells) {
        int index = -1;
        float verticalMidpoint = bounds.left + bounds.width / 2;
        float horizontalMidpoint = bounds.top + bounds.height / 2;

        bool topQuadrant = (rect.top < horizontalMidpoint && rect.top + rect.height < horizontalMidpoint);
        bool bottomQuadrant = (rect.top > horizontalMidpoint);

        if (rect.left < verticalMidpoint && rect.left + rect.width < verticalMidpoint) {
            if (topQuadrant)
                index = 0; // Top-left
            else if (bottomQuadrant)
                index = 2; // Bottom-left
        }
        else if (rect.left > verticalMidpoint) {
            if (topQuadrant)
                index = 1; // Top-right
            else if (bottomQuadrant)
                index = 3; // Bottom-right
        }
        return index;
    }

    void insert(int rectIndex, std::vector<Cell>& cells) {
        if (nodes[0] != nullptr) {
            int index = getIndex(cells[rectIndex].shape.getGlobalBounds(), cells);
            if (index != -1) {
                nodes[index]->insert(rectIndex, cells);
                return;
            }
        }

        objectIndices.push_back(rectIndex);

        if (objectIndices.size() > maxObjects && level < maxLevels) {
            if (nodes[0] == nullptr) {
                split();
            }

            auto it = objectIndices.begin();
            while (it != objectIndices.end()) {
                int index = getIndex(cells[*it].shape.getGlobalBounds(), cells);
                if (index != -1) {
                    nodes[index]->insert(*it, cells);
                    it = objectIndices.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
    }

    std::vector<int> retrieve(const sf::FloatRect& rect, const std::vector<Cell>& cells) {
        std::vector<int> result;
        int index = getIndex(rect, cells);

        if (index != -1 && nodes[0] != nullptr) {
            result = nodes[index]->retrieve(rect, cells);
        }

        for (auto& objectIndex : objectIndices) {
            result.push_back(objectIndex);
        }

        return result;
    }
};

int getIndex(int row, int col, int width) {
    return row * width + col;
}

std::vector<int> getNeighbors(int index, int width, int height) {
    int row = index / width;
    int col = index % width;

    std::vector<int> neighbors;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue; // Skip the current cell itself

            int newRow = row + dr;
            int newCol = col + dc;

            // Boundary check
            if (newRow >= 0 && newRow < height && newCol >= 0 && newCol < width) {
                neighbors.push_back(getIndex(newRow, newCol, width));
            }
        }
    }

    return neighbors;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "SFML works!");
    sf::View view(sf::FloatRect(0, 0, windowWidth, windowHeight));

    std::vector<Cell> cells;
    Quadtree quadtree(sf::FloatRect(0, 0, windowWidth, windowHeight));

    int numCellsX = windowWidth / cellSize;
    int numCellsY = windowHeight / cellSize;

    for (int i = 0; i < numCellsX; i++) {
        for (int r = 0; r < numCellsY; r++) {
            Cell c(sf::Vector2f(i * cellSize, r * cellSize), sf::Vector2f(cellSize, cellSize));

            cells.push_back(c);
            quadtree.insert(cells.size() - 1, cells);
        }
    }

    bool isDragging = false;
    sf::RectangleShape* hoveredRect = nullptr;
    std::vector<int> cursorPosition;
    cursorPosition.push_back(-99999);
    cursorPosition.push_back(-99999);

    while (window.isOpen()) {
        sf::Event event;

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePosition);
        sf::FloatRect mouseRect(mouseWorldPos.x, mouseWorldPos.y, 1, 1);

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                isDragging = true;
            }

            // Detect when the mouse button is released
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                isDragging = false;
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    view.zoom(0.9f);
                }
                else if (event.mouseWheelScroll.delta < 0) {
                    view.zoom(1.1f);
                }

                cells.clear();
                quadtree.clear();

                for (int i = 0; i < numCellsX; i++) {
                    for (int r = 0; r < numCellsY; r++) {
                        Cell c(sf::Vector2f(i * cellSize, r * cellSize), sf::Vector2f(cellSize, cellSize));

                        cells.push_back(c);
                        quadtree.insert(cells.size() - 1, cells);
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved) {
                if (cursorPosition[0] != -99999 && isDragging == true) {
                    view.move(cursorPosition[0] - mousePosition.x, cursorPosition[1] - mousePosition.y);
                }
                cursorPosition[0] = mousePosition.x;
                cursorPosition[1] = mousePosition.y;
            }

            if (event.type == sf::Event::Resized) {
                view.setSize(sf::Vector2f(event.size.width, event.size.height));
            }
        }

        // Retrieve rectangles potentially affected
        auto closeRects = quadtree.retrieve(mouseRect, cells);

        // Reset all rectangles to black
        for (auto& rect : cells) {
            rect.shape.setFillColor(sf::Color(0, 0, 0));
        }

        // Set color to red if hovered
        for (auto rectIndex : closeRects) {
            if (cells[rectIndex].shape.getGlobalBounds().contains(mouseWorldPos.x, mouseWorldPos.y)) {
                std::vector<int> n = getNeighbors(rectIndex, numCellsX, numCellsY);
                std::cout << "---" << std::endl;
                for (auto i : n) {
                    std::cout << i << std::endl;
                    cells[i].shape.setFillColor(sf::Color(255, 0, 0));
                }
                std::cout << "---" << std::endl;
                cells[rectIndex].shape.setFillColor(sf::Color(255, 255, 255));
            }
        }

        // Clear the window and draw all rectangles
        window.clear();
        window.setView(view);
        for (const auto& rect : cells) {
            window.draw(rect.shape);
        }
        window.display();
    }

    return 0;
}
