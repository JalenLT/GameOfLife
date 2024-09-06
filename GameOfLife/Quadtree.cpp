#include "Quadtree.hpp"

Quadtree::Quadtree(sf::FloatRect bounds, int level, int maxObjects, int maxLevels)
    : bounds(bounds), level(level), maxObjects(maxObjects), maxLevels(maxLevels) {
    for (int i = 0; i < 4; i++) {
        nodes[i] = nullptr;
    }
}

Quadtree::~Quadtree() {
    for (int i = 0; i < 4; i++) {
        delete nodes[i];
    }
}

void Quadtree::clear() {
    objectIndices.clear();
    if (nodes[0] != nullptr) {
        for (int i = 0; i < 4; i++) {
            nodes[i]->clear();
        }
    }
}

void Quadtree::split() {
    float subWidth = bounds.width / 2;
    float subHeight = bounds.height / 2;

    nodes[0] = new Quadtree(sf::FloatRect(bounds.left, bounds.top, subWidth, subHeight), level + 1, maxObjects, maxLevels);
    nodes[1] = new Quadtree(sf::FloatRect(bounds.left + subWidth, bounds.top, subWidth, subHeight), level + 1, maxObjects, maxLevels);
    nodes[2] = new Quadtree(sf::FloatRect(bounds.left, bounds.top + subHeight, subWidth, subHeight), level + 1, maxObjects, maxLevels);
    nodes[3] = new Quadtree(sf::FloatRect(bounds.left + subWidth, bounds.top + subHeight, subWidth, subHeight), level + 1, maxObjects, maxLevels);
}

int Quadtree::getIndex(const sf::FloatRect& rect, const std::vector<Cell>& cells) {
    int index = -1;
    float verticalMidpoint = bounds.left + bounds.width / 2;
    float horizontalMidpoint = bounds.top + bounds.height / 2;

    bool topQuadrant = (rect.top < horizontalMidpoint && rect.top + rect.height < horizontalMidpoint);
    bool bottomQuadrant = (rect.top > horizontalMidpoint);

    if (rect.left < verticalMidpoint && rect.left + rect.width < verticalMidpoint) {
        if (topQuadrant)
            index = 0;
        else if (bottomQuadrant)
            index = 2;
    }
    else if (rect.left > verticalMidpoint) {
        if (topQuadrant)
            index = 1;
        else if (bottomQuadrant)
            index = 3;
    }
    return index;
}

void Quadtree::insert(int rectIndex, std::vector<Cell>& cells) {
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

std::vector<int> Quadtree::retrieve(const sf::FloatRect& rect, const std::vector<Cell>& cells) {
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
