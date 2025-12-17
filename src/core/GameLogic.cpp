#include "GameLogic.hpp"
#include <algorithm> // For std::reverse
#include <vector>

namespace Core {

GameLogic::MoveResult GameLogic::move(Grid &grid, Direction dir) {
  // Pre-move: Reset merge flags from previous turn
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      grid.getTile(x, y).resetMerged();
    }
  }

  bool changed = false;
  int totalScore = 0;

  // 1. Transform Grid to canonical "Left" orientation
  switch (dir) {
  case Direction::Left:
    // No transform needed
    break;
  case Direction::Right:
    reverseGrid(grid);
    break;
  case Direction::Up:
    transposeGrid(grid);
    break;
  case Direction::Down:
    transposeGrid(grid);
    reverseGrid(grid); // Transpose + Reverse = Rotate 90 deg
    break;
  }

  // 2. Process all rows using "Slide Left" logic
  // Now that the grid is transformed, every "row" is what we want to slide
  // left.
  for (int y = 0; y < 4; ++y) {
    // We need to extract the row to pass it to our helper.
    // Since we can't easily get a reference to a "Row" from the Grid class
    // (because Grid encapsulates the array), we might need to iterate.
    // BUT, our "Grid" class implementation uses `std::array<std::array<...>>`.
    // We can access tiles directly.

    // Let's extract row data for processing
    std::array<Tile, 4> tempRow;
    for (int x = 0; x < 4; ++x)
      tempRow[x] = grid.getTile(x, y);

    auto result = slideAndMergeRow(tempRow);
    if (result.first) {
      changed = true;
      totalScore += result.second;
      // Write back
      for (int x = 0; x < 4; ++x)
        grid.getTile(x, y) = tempRow[x];
    }
  }

  // 3. Inverse Transform (Restore original orientation)
  switch (dir) {
  case Direction::Left:
    break;
  case Direction::Right:
    reverseGrid(grid);
    break;
  case Direction::Up:
    transposeGrid(grid);
    break;
  case Direction::Down:
    reverseGrid(grid); // Reverse back first
    transposeGrid(grid);
    break;
  }

  return {changed, totalScore};
}

std::pair<bool, int> GameLogic::slideAndMergeRow(std::array<Tile, 4> &row) {
  bool rowChanged = false;
  int score = 0;

  // Phase 1: Compression (Move non-zeros to temp)
  std::vector<int> buffer;
  buffer.reserve(4);
  for (const auto &t : row) {
    if (!t.isEmpty()) {
      buffer.push_back(t.getValue());
    }
  }

  // Phase 2: Merge & Reconstruct
  std::vector<Tile> mergedResult;
  mergedResult.reserve(4);

  for (size_t i = 0; i < buffer.size(); ++i) {
    if (i < buffer.size() - 1 && buffer[i] == buffer[i + 1]) {
      // MERGE: Combine current and next
      int newVal = buffer[i] * 2;
      score += newVal; // Add Logic for Scoring

      Tile t(newVal);
      t.setMerged(true);
      mergedResult.push_back(t);
      i++; // Skip next tile (it's consumed)
    } else {
      // KEEP: Just copy current
      mergedResult.push_back(Tile(buffer[i]));
    }
  }

  // Phase 3: Fill remaining with empty
  while (mergedResult.size() < 4) {
    mergedResult.push_back(Tile(0));
  }

  // Compare with original to detect change & Write Back
  for (int i = 0; i < 4; ++i) {
    if (row[i].getValue() != mergedResult[i].getValue()) {
      rowChanged = true;
    }
    row[i] = mergedResult[i];
  }

  return {rowChanged, score};
}

bool GameLogic::isGameOver(const Grid &grid) const {
  // 1. Check for empty tiles
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      if (grid.getTile(x, y).isEmpty()) {
        return false; // Empty slot exists -> Playable
      }
    }
  }

  // 2. Check for possible merges (Horizontal)
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 3; ++x) {
      if (grid.getTile(x, y).getValue() == grid.getTile(x + 1, y).getValue()) {
        return false; // Merge possible
      }
    }
  }

  // 3. Check for possible merges (Vertical)
  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 3; ++y) {
      if (grid.getTile(x, y).getValue() == grid.getTile(x, y + 1).getValue()) {
        return false; // Merge possible
      }
    }
  }

  return true; // Full and no merges
}

void GameLogic::reverseGrid(Grid &grid) {
  for (int y = 0; y < 4; ++y) {
    // We need to swap [x] with [3-x]
    for (int x = 0; x < 2; ++x) {
      std::swap(grid.getTile(x, y), grid.getTile(3 - x, y));
    }
  }
}

void GameLogic::transposeGrid(Grid &grid) {
  for (int y = 0; y < 4; ++y) {
    for (int x = y + 1; x < 4; ++x) {
      std::swap(grid.getTile(x, y), grid.getTile(y, x));
    }
  }
}

} // namespace Core
