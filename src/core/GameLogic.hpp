#pragma once
#include "Grid.hpp"
#include <vector>

namespace Core {

enum class Direction { Up, Down, Left, Right };

class GameLogic {
public:
  struct MoveResult {
    bool moved;
    int score;
  };

  /**
   * @brief Executes a move on the grid.
   *
   * @param grid The game grid to modify.
   * @param dir The direction to slide tiles.
   * @return MoveResult containing success flag and points earned.
   */
  MoveResult move(Grid &grid, Direction dir);

  // Checks if the game is over (no empty slots and no adjacent merges)
  bool isGameOver(const Grid &grid) const;

private:
  // Helper to process a single row (Slide Left logic)
  // Returns pair: { changed, score_gained }
  std::pair<bool, int> slideAndMergeRow(std::array<Tile, 4> &row);

  // Helpers for Transformation Strategy
  void reverseGrid(Grid &grid);
  void transposeGrid(Grid &grid);
};

} // namespace Core
