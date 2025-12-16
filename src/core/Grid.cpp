#include "Grid.hpp"
#include <random>
#include <vector>

namespace Core {

Grid::Grid() {
  // Initialize RNG with a random device seed
  std::random_device rd;
  rng.seed(rd());

  reset();
}

void Grid::reset() {
  for (auto &row : tiles) {
    for (auto &tile : row) {
      tile = Tile(0); // Replace with empty tile
    }
  }
}

const Tile &Grid::getTile(int x, int y) const {
  // In a real production engine, we might add bounds checking (asserts)
  return tiles[y][x]; // Row-major: tiles[row][col] -> tiles[y][x]
}

Tile &Grid::getTile(int x, int y) { return tiles[y][x]; }

bool Grid::spawnRandomTile() {
  struct Coordinate {
    int x;
    int y;
  };
  std::vector<Coordinate> emptySlots;
  emptySlots.reserve(16);

  // 1. Find all empty slots
  for (int y = 0; y < SIZE; ++y) {
    for (int x = 0; x < SIZE; ++x) {
      if (tiles[y][x].isEmpty()) {
        emptySlots.push_back({x, y});
      }
    }
  }

  if (emptySlots.empty()) {
    return false;
  }

  // 2. Pick a random slot
  std::uniform_int_distribution<int> slotDist(
      0, static_cast<int>(emptySlots.size()) - 1);
  Coordinate choice = emptySlots[slotDist(rng)];

  // 3. Pick a value (10% chance for 4, 90% chance for 2)
  std::uniform_int_distribution<int> valueProb(0, 9);
  int value = (valueProb(rng) < 1) ? 4 : 2;

  tiles[choice.y][choice.x] = Tile(value);
  return true;
}

} // namespace Core
