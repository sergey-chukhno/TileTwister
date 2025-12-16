#include "GameLogic.hpp"
#include "Grid.hpp"
#include <gtest/gtest.h>

// Helper to set a row for easy testing
void setRow(Core::Grid &grid, int rowY, std::vector<int> values) {
  for (int x = 0; x < 4; ++x) {
    if (x < values.size()) {
      grid.getTile(x, rowY) = Core::Tile(values[x]);
    } else {
      grid.getTile(x, rowY) = Core::Tile(0);
    }
  }
}

// Helper to check a row
void checkRow(const Core::Grid &grid, int rowY, std::vector<int> expected) {
  for (int x = 0; x < 4; ++x) {
    int val = (x < expected.size()) ? expected[x] : 0;
    EXPECT_EQ(grid.getTile(x, rowY).getValue(), val)
        << "Mismatch at Row " << rowY << ", Col " << x;
  }
}

class GameLogicTest : public ::testing::Test {
protected:
  Core::Grid grid;
  Core::GameLogic logic;
};

TEST_F(GameLogicTest, SlideLeft_SimpleSlide) {
  // [0, 2, 0, 4] -> [2, 4, 0, 0] (No merge, just move)
  setRow(grid, 0, {0, 2, 0, 4});

  bool moved = logic.move(grid, Core::Direction::Left);

  EXPECT_TRUE(moved);
  checkRow(grid, 0, {2, 4, 0, 0});
}

TEST_F(GameLogicTest, SlideLeft_SimpleMerge) {
  // [2, 2, 0, 0] -> [4, 0, 0, 0]
  setRow(grid, 0, {2, 2, 0, 0});

  bool moved = logic.move(grid, Core::Direction::Left);

  EXPECT_TRUE(moved);
  checkRow(grid, 0, {4, 0, 0, 0});
  EXPECT_TRUE(grid.getTile(0, 0).hasMerged());
}

TEST_F(GameLogicTest, SlideLeft_MergePriority) {
  // [2, 2, 2, 0] -> [4, 2, 0, 0] NOT [2, 4, 0, 0]
  setRow(grid, 0, {2, 2, 2, 0});

  logic.move(grid, Core::Direction::Left);

  checkRow(grid, 0, {4, 2, 0, 0});
}

TEST_F(GameLogicTest, SlideLeft_DoubleMerge) {
  // [2, 2, 2, 2] -> [4, 4, 0, 0]
  setRow(grid, 0, {2, 2, 2, 2});

  logic.move(grid, Core::Direction::Left);

  checkRow(grid, 0, {4, 4, 0, 0});
}

TEST_F(GameLogicTest, SlideLeft_SlideAndMerge) {
  // [2, 0, 2, 2] -> [4, 2, 0, 0]
  // 1. Slide: [2, 2, 2, 0]
  // 2. Merge: [4, 2, 0, 0]
  setRow(grid, 0, {2, 0, 2, 2});

  logic.move(grid, Core::Direction::Left);

  checkRow(grid, 0, {4, 2, 0, 0});
}

TEST_F(GameLogicTest, SlideRight_ReverseLogic) {
  // [2, 2, 0, 0] -> [0, 0, 0, 4]
  setRow(grid, 0, {2, 2, 0, 0});

  bool moved = logic.move(grid, Core::Direction::Right);

  EXPECT_TRUE(moved);
  checkRow(grid, 0, {0, 0, 0, 4});
}

TEST_F(GameLogicTest, SlideUp_TransposeLogic) {
  // Col 0: [2, 2, 0, 0]T -> [4, 0, 0, 0]T
  grid.getTile(0, 0) = Core::Tile(2);
  grid.getTile(0, 1) = Core::Tile(2);

  bool moved = logic.move(grid, Core::Direction::Up);

  EXPECT_TRUE(moved);
  EXPECT_EQ(grid.getTile(0, 0).getValue(), 4);
  EXPECT_EQ(grid.getTile(0, 1).getValue(), 0);
}

TEST_F(GameLogicTest, NoMoveReturnsFalse) {
  // [2, 4, 8, 16] -> No moves possible Left
  setRow(grid, 0, {2, 4, 8, 16});

  bool moved = logic.move(grid, Core::Direction::Left);

  EXPECT_FALSE(moved);
}

TEST_F(GameLogicTest, SlideRight_Merge) {
  // [2, 2, 0, 0] -> [0, 0, 0, 4]
  setRow(grid, 0, {2, 2, 0, 0});

  bool moved = logic.move(grid, Core::Direction::Right);

  EXPECT_TRUE(moved);
  checkRow(grid, 0, {0, 0, 0, 4});
}

TEST_F(GameLogicTest, SlideDown_Merge) {
  // Col 0: [2, 2, 0, 0]T (Top is 0,0) -> [0, 0, 0, 4]T (Bottom is 3,0)
  grid.getTile(0, 0) = Core::Tile(2);
  grid.getTile(0, 1) = Core::Tile(2);
  grid.getTile(0, 2) = Core::Tile(0);
  grid.getTile(0, 3) = Core::Tile(0);

  bool moved = logic.move(grid, Core::Direction::Down);

  EXPECT_TRUE(moved);
  // After Down move, 4 should be at (0, 3)
  EXPECT_EQ(grid.getTile(0, 3).getValue(), 4);
  EXPECT_EQ(grid.getTile(0, 2).getValue(), 0);
  EXPECT_EQ(grid.getTile(0, 0).getValue(), 0);
}

TEST_F(GameLogicTest, SlideDown_Complex) {
  // Col 0: [2, 0, 2, 2] -> [0, 2, 0, 4] ? No:
  // Compress Down: [0, 2, 2, 2]
  // Merge Down:    [0, 0, 2, 4]

  grid.getTile(0, 0) = Core::Tile(2);
  grid.getTile(0, 1) = Core::Tile(0);
  grid.getTile(0, 2) = Core::Tile(2);
  grid.getTile(0, 3) = Core::Tile(2);

  bool moved = logic.move(grid, Core::Direction::Down);

  EXPECT_TRUE(moved);
  EXPECT_EQ(grid.getTile(0, 3).getValue(), 4);
  EXPECT_EQ(grid.getTile(0, 2).getValue(), 2);
  EXPECT_EQ(grid.getTile(0, 1).getValue(), 0);
}
