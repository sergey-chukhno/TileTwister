#pragma once
#include "Grid.hpp"
#include <vector>

namespace Core {

enum class Direction { Up, Down, Left, Right };

class GameLogic {
public:
  /**
   * @brief Executes a move on the grid.
   *
   * @param grid The game grid to modify.
   * @param dir The direction to slide tiles.
   * @return true if the grid changed (valid move), false otherwise.
   */
  bool move(Grid &grid, Direction dir);

  // Checks if the game is over (no empty slots and no adjacent merges)
  // We'll implement this later in the Game Loop phase,
  // but good to declare intent now.
  // bool isGameOver(const Grid& grid) const;

private:
  // Helper to process a single row (Slide Left logic)
  // Returns true if the row changed
  bool slideAndMergeRow(std::array<Tile, 4> &row);

  // Helpers for Transformation Strategy
  void reverseGrid(Grid &grid);
  void transposeGrid(Grid &grid);
};

} // namespace Core
