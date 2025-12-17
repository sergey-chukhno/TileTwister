#include "Game.hpp"
#include <SDL.h>
#include <iostream>

namespace Game {

Game::Game()
    : m_context(), m_window("Tile Twister - 2048", WINDOW_WIDTH, WINDOW_HEIGHT),
      m_renderer(m_window), m_grid(), m_logic(), m_isRunning(true) {
  // Initial Setup
  m_grid.reset();
  m_grid.spawnRandomTile();
  m_grid.spawnRandomTile();
}

void Game::run() {
  std::cout << "Game Loop Started." << std::endl;

  while (m_isRunning) {
    handleInput();
    update();
    render();

    SDL_Delay(16); // Cap at ~60 FPS
  }

  std::cout << "Game Loop Ended." << std::endl;
}

void Game::handleInput() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      m_isRunning = false;
    } else if (e.type == SDL_KEYDOWN) {
      bool moved = false;
      switch (e.key.keysym.sym) {
      case SDLK_UP:
        moved = m_logic.move(m_grid, Core::Direction::Up);
        break;
      case SDLK_DOWN:
        moved = m_logic.move(m_grid, Core::Direction::Down);
        break;
      case SDLK_LEFT:
        moved = m_logic.move(m_grid, Core::Direction::Left);
        break;
      case SDLK_RIGHT:
        moved = m_logic.move(m_grid, Core::Direction::Right);
        break;
      case SDLK_ESCAPE:
        m_isRunning = false;
        break;
      case SDLK_r: // Debug Reset
        m_grid.reset();
        m_grid.spawnRandomTile();
        m_grid.spawnRandomTile();
        break;
      }

      if (moved) {
        m_grid.spawnRandomTile();
        std::cout << "Move Successful. Spawning Tile." << std::endl;
      }
    }
  }
}

void Game::update() {
  // Logic independent of input
}

void Game::render() {
  // 1. Background
  m_renderer.setDrawColor(250, 248, 239); // Light Beige
  m_renderer.clear();

  // 2. Render Grid
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      const auto &tile = m_grid.getTile(x, y);

      SDL_Rect rect = getTileRect(x, y);
      Color c = getTileColor(tile.getValue());

      m_renderer.setDrawColor(c.r, c.g, c.b, c.a);
      m_renderer.drawFillRect(rect.x, rect.y, rect.w, rect.h);
    }
  }

  m_renderer.present();
}

Color Game::getTileColor(int val) const {
  switch (val) {
  case 0:
    return {205, 193, 180, 255}; // Empty Gray
  case 2:
    return {238, 228, 218, 255}; // Off-white
  case 4:
    return {237, 224, 200, 255}; // Beige
  case 8:
    return {242, 177, 121, 255}; // Orange
  case 16:
    return {245, 149, 99, 255}; // Dark Orange
  case 32:
    return {246, 124, 95, 255}; // Red-Orange
  case 64:
    return {246, 94, 59, 255}; // Red
  case 128:
    return {237, 207, 114, 255}; // Yellow
  case 256:
    return {237, 204, 97, 255}; // Yellow-Gold
  case 512:
    return {237, 200, 80, 255};
  case 1024:
    return {237, 197, 63, 255};
  case 2048:
    return {237, 194, 46, 255};
  default:
    return {60, 58, 50, 255}; // Black/Super high
  }
}

SDL_Rect Game::getTileRect(int x, int y) const {
  SDL_Rect rect;
  rect.w = TILE_SIZE;
  rect.h = TILE_SIZE;
  rect.x = GRID_OFFSET_X + (x * (TILE_SIZE + GRID_PADDING));
  rect.y = GRID_OFFSET_Y + (y * (TILE_SIZE + GRID_PADDING));
  return rect;
}

} // namespace Game
