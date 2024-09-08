#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>
#include "Cell.hpp"
#include "Quadtree.hpp"

const int windowWidth{ 800 };
const int windowHeight{ 800 };
const float cellSize{ 30 };
const int cellThickness{ 3 };

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

    std::vector<std::vector<std::vector<int>>> timeline;
    bool redraw = false;

    int numCellsX = windowWidth / cellSize;
    int numCellsY = windowHeight / cellSize;

    std::vector<int> frameCell;
    std::vector<std::vector<int>> frame;
    for (int i = 0; i < numCellsX; i++) {
        for (int r = 0; r < numCellsY; r++) {
            frameCell.clear();
            Cell c(sf::Vector2f(i * cellSize, r * cellSize), sf::Vector2f(cellSize, cellSize));

            cells.push_back(c);
            quadtree.insert(cells.size() - 1, cells);
            frameCell.push_back(0);
            frameCell.push_back(c.isAlive);
            frame.push_back(frameCell);
        }
    }
    timeline.push_back(frame);

    std::map<std::string, int> currentEvents;
    currentEvents["holdingLeftClick"] = false;
    currentEvents["holdingRightClick"] = false;

    std::string cellHeldState = "";

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
                currentEvents["holdingLeftClick"] = true;
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                currentEvents["holdingLeftClick"] = false;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                currentEvents["holdingRightClick"] = true;
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
                cellHeldState = "";
                currentEvents["holdingRightClick"] = false;
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Right) {
                redraw = true;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Left && timeline.size() > 1) {
                timeline.pop_back();
                auto last = timeline[timeline.size() - 1];
                //DRAW BOARD
                for (int i = 0; i < last.size(); i++)
                {
                    cells[i].isAlive = last[i][1];
                }
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
                if (cursorPosition[0] != -99999 && currentEvents["holdingLeftClick"] == true) {
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
        auto& last = timeline.back();

        for (auto rectIndex : closeRects) {
            if (cells[rectIndex].shape.getGlobalBounds().contains(mouseWorldPos.x, mouseWorldPos.y)) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && cellHeldState == "") {
                    if (cells[rectIndex].isAlive) {
                        cellHeldState = "dead";
                    }
                    else if (!cells[rectIndex].isAlive) {
                        cellHeldState = "alive";
                    }
                }

                //std::vector<int> n = getNeighbors(rectIndex, numCellsX, numCellsY);

                if (currentEvents["holdingRightClick"]) {
                    if (cellHeldState == "alive") {
                        last[rectIndex][1] = 1;
                        cells[rectIndex].isAlive = true;
                    }
                    else if (cellHeldState == "dead") {
                        last[rectIndex][1] = 0;
                        cells[rectIndex].isAlive = false;
                    }
                }
            }
        }

        if (redraw) {
            std::vector<int> frameCell;
            std::vector<std::vector<int>> frame;

            // Reset all rectangles to black
            for (size_t i = 0; i < cells.size(); ++i) {
                frameCell.clear();
                auto& rect = cells[i];  // Access the rect at index i
                std::vector<int> neighbours = getNeighbors(i, numCellsX, numCellsY);
                int liveNeighbours = 0;

                /**
                    RULES
                    -----
                    - Any live cell with fewer than two live neighbours dies, as if by underpopulation
                    - Any live cell with two or three live neighbours lives on to the next generation
                    - Any live cell with more than three live neighbours dies, as if by overpopulation
                    - Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction
                */

                for (int r = 0; r < neighbours.size(); r++)
                {
                    if (last[neighbours[r]][1])
                        liveNeighbours += 1;
                }
                
                if (rect.isAlive == true && liveNeighbours < 2) {
                    rect.isAlive = false;
                }
                else if (rect.isAlive == true && (liveNeighbours == 2 || liveNeighbours == 3)) {
                    rect.isAlive = true; 
                }
                else if (rect.isAlive == true && liveNeighbours > 3) {
                    rect.isAlive = false;
                }
                else if (rect.isAlive == false && liveNeighbours == 3) {
                    rect.isAlive = true;
                }

                if (rect.isAlive) {
                    rect.shape.setFillColor(sf::Color(255, 255, 255));
                }
                else {
                    rect.shape.setFillColor(sf::Color(0, 0, 0));
                }
                frameCell.push_back(i);
                frameCell.push_back(rect.isAlive);
                frame.push_back(frameCell);
            }
            timeline.push_back(frame);
            redraw = false;
        }
        else {
            // Reset all rectangles to black
            for (size_t i = 0; i < cells.size(); ++i) {
                auto& rect = cells[i];  // Access the rect at index i
                if (rect.isAlive) {
                    rect.shape.setFillColor(sf::Color(255, 255, 255));
                }
                else {
                    rect.shape.setFillColor(sf::Color(0, 0, 0));
                }
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
