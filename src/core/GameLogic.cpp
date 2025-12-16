#include "GameLogic.hpp"
#include <algorithm> // For std::reverse
#include <vector>

namespace Core {

bool GameLogic::move(Grid &grid, Direction dir) {
  // Pre-move: Reset merge flags from previous turn
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      grid.getTile(x, y).resetMerged();
    }
  }

  bool changed = false;

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
    // Wait, standard matrix rotation is more complex,
    // but Transpose -> Reverse Rows gives us "Down" as "Left"?
    // Let's trace:
    // Original:
    // [1 2]
    // [3 4]
    // Down move -> 3 moves to 4's row.
    // Transpose:
    // [1 3]
    // [2 4]
    // Reverse Rows:
    // [3 1]
    // [4 2]
    // Slide Left:
    // [3 1] (No move)
    // Reverse Back:
    // [1 3]
    // Transpose Back:
    // [1 2]
    // [3 4]
    // Actually, for Down (Bottom-to-Top push), we want Transpose -> Reverse.
    // Let's verify with the Inverse at the end.
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

    if (slideAndMergeRow(tempRow)) {
      changed = true;
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

  return changed;
}

bool GameLogic::slideAndMergeRow(std::array<Tile, 4> &row) {
  bool rowChanged = false;

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
      Tile t(buffer[i] * 2);
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

  // Note: If we just merged (2,2 -> 4,0), values changed.
  // If we just slid (0,2 -> 2,0), values changed.
  // If we didn't move (2,0,0,0), values same.

  return rowChanged;
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
