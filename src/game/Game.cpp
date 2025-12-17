#include "Game.hpp"
#include <SDL.h>
#include <iostream>
#include <string>

namespace Game {

Game::Game()
    : m_context(), m_window("Tile Twister - 2048", WINDOW_WIDTH, WINDOW_HEIGHT),
      m_renderer(m_window), m_font("assets/font.ttf", 55), m_inputManager(),
      m_grid(), m_logic(), m_isRunning(true), m_state(GameState::MainMenu),
      m_menuSelection(0), m_darkSkin(false), m_soundOn(true), m_score(0),
      m_bestScore(0) {
  // Initial Setup not needed for Menu start, but good to have ready
  resetGame();
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
  Action action = m_inputManager.pollAction();

  if (action == Action::Quit) {
    m_isRunning = false;
    return;
  }

  switch (m_state) {
  case GameState::MainMenu:
    if (action == Action::Up) {
      m_menuSelection--;
      if (m_menuSelection < 0)
        m_menuSelection = 5; // 6 Items (0-5)
    } else if (action == Action::Down) {
      m_menuSelection++;
      if (m_menuSelection > 5)
        m_menuSelection = 0;
    } else if (action == Action::Confirm) {
      switch (m_menuSelection) {
      case 0: // Start
        resetGame();
        m_state = GameState::Playing;
        break;
      case 1: // Load
        m_state = GameState::LoadGame;
        m_menuSelection = 0;
        break;
      case 2: // Options
        m_state = GameState::Options;
        m_menuSelection = 0;
        break;
      case 3: // Leaderboard
        m_state = GameState::Leaderboard;
        m_menuSelection = 0;
        break;
      case 4: // Achievements
        m_state = GameState::Achievements;
        m_menuSelection = 0;
        break;
      case 5: // Quit
        m_isRunning = false;
        break;
      }
    }
    break;

  case GameState::Playing:
    handleInputPlaying(action);
    break;

  case GameState::Options:
    handleInputOptions(action);
    break;

  case GameState::Leaderboard:
  case GameState::Achievements:
  case GameState::LoadGame:
    handleInputPlaceholder(action);
    break;

  case GameState::GameOver:
    if (action == Action::Up || action == Action::Down) {
      m_menuSelection = !m_menuSelection;
    } else if (action == Action::Confirm) {
      if (m_menuSelection == 0) { // Restart
        resetGame();
        m_state = GameState::Playing;
      } else { // Return to Menu
        m_state = GameState::MainMenu;
        m_menuSelection = 0;
      }
    }
    break;
  }
}

void Game::handleInputOptions(Action action) {
  if (action == Action::Up) {
    m_menuSelection--;
    if (m_menuSelection < 0)
      m_menuSelection = 3; // 3 Options (Theme, Sound, Controls, Back) -> Wait,
                           // 4 items
  } else if (action == Action::Down) {
    m_menuSelection++;
    if (m_menuSelection > 3)
      m_menuSelection = 0;
  } else if (action == Action::Confirm ||
             (action == Action::Right || action == Action::Left)) {
    // Left/Right or Enter can toggle booleans
    switch (m_menuSelection) {
    case 0:
      m_darkSkin = !m_darkSkin;
      break;
    case 1:
      m_soundOn = !m_soundOn;
      break;
    case 2:
      break; // Controls is just info
    case 3:  // Back
      m_state = GameState::MainMenu;
      m_menuSelection = 2; // Return to "Options" highlighted
      break;
    }
  } else if (action == Action::Back) {
    m_state = GameState::MainMenu;
    m_menuSelection = 2;
  }
}

void Game::handleInputPlaceholder(Action action) {
  if (action == Action::Confirm || action == Action::Back) {
    m_state = GameState::MainMenu;
    m_menuSelection = 0;
  }
}

void Game::handleInputPlaying(Action action) {
  Core::GameLogic::MoveResult result = {false, 0};
  switch (action) {
  case Action::Up:
    result = m_logic.move(m_grid, Core::Direction::Up);
    break;
  case Action::Down:
    result = m_logic.move(m_grid, Core::Direction::Down);
    break;
  case Action::Left:
    result = m_logic.move(m_grid, Core::Direction::Left);
    break;
  case Action::Right:
    result = m_logic.move(m_grid, Core::Direction::Right);
    break;
  case Action::Back:
    m_state = GameState::MainMenu;
    m_menuSelection = 0;
    return;
  case Action::Restart:
    resetGame();
    return;
  default:
    break;
  }

  if (result.moved) {
    m_score += result.score;
    if (m_score > m_bestScore)
      m_bestScore = m_score;
    m_grid.spawnRandomTile();

    if (m_logic.isGameOver(m_grid)) {
      m_state = GameState::GameOver;
      m_menuSelection = 0; // Reset selection for Game Over menu
    }
  }
}

void Game::update() {}

void Game::render() {
  // 1. Background (Theme Aware)
  if (m_darkSkin) {
    m_renderer.setDrawColor(30, 30, 30); // Dark Gray
  } else {
    m_renderer.setDrawColor(250, 248, 239); // Light Beige
  }
  m_renderer.clear();

  switch (m_state) {
  case GameState::MainMenu:
    renderMenu();
    break;
  case GameState::Playing:
    renderPlaying();
    break;
  case GameState::GameOver:
    renderGameOver();
    break;
  case GameState::Options:
    renderOptions();
    break;
  case GameState::LoadGame:
    renderPlaceholder("LOAD GAME");
    break;
  case GameState::Leaderboard:
    renderPlaceholder("LEADERBOARD");
    break;
  case GameState::Achievements:
    renderPlaceholder("ACHIEVEMENTS");
    break;
  }

  m_renderer.present();
}

void Game::renderMenu() {
  m_renderer.drawTextCentered("TILE TWISTER", m_font, WINDOW_WIDTH / 2, 50,
                              m_darkSkin ? 255 : 119, m_darkSkin ? 255 : 110,
                              m_darkSkin ? 255 : 101, 255);

  const char *options[] = {"Start Game",  "Load Game",    "Options",
                           "Leaderboard", "Achievements", "Quit"};
  int y = 150;

  for (int i = 0; i < 6; ++i) {
    uint8_t alpha = (m_menuSelection == i) ? 255 : 100;
    uint8_t r = m_darkSkin ? 255 : 119;
    uint8_t g = m_darkSkin ? 255 : 110;
    uint8_t b = m_darkSkin ? 255 : 101;

    m_renderer.drawTextCentered(options[i], m_font, WINDOW_WIDTH / 2, y, r, g,
                                b, alpha);
    y += 60;
  }
}

void Game::renderOptions() {
  m_renderer.drawTextCentered("OPTIONS", m_font, WINDOW_WIDTH / 2, 50,
                              m_darkSkin ? 255 : 119, m_darkSkin ? 255 : 110,
                              m_darkSkin ? 255 : 101, 255);

  int y = 200;
  uint8_t r = m_darkSkin ? 255 : 119;
  uint8_t g = m_darkSkin ? 255 : 110;
  uint8_t b = m_darkSkin ? 255 : 101;

  // 0: Theme
  std::string themeText =
      std::string("Skin: ") + (m_darkSkin ? "Dark" : "Light");
  m_renderer.drawTextCentered(themeText, m_font, WINDOW_WIDTH / 2, y, r, g, b,
                              (m_menuSelection == 0) ? 255 : 100);
  y += 60;

  // 1: Sound
  std::string soundText = std::string("Sound: ") + (m_soundOn ? "On" : "Off");
  m_renderer.drawTextCentered(soundText, m_font, WINDOW_WIDTH / 2, y, r, g, b,
                              (m_menuSelection == 1) ? 255 : 100);
  y += 60;

  // 2: Controls
  m_renderer.drawTextCentered("Controls: WASD/Arrows", m_font, WINDOW_WIDTH / 2,
                              y, r, g, b, (m_menuSelection == 2) ? 255 : 100);
  y += 60;

  // 3: Back
  m_renderer.drawTextCentered("Back", m_font, WINDOW_WIDTH / 2, y, r, g, b,
                              (m_menuSelection == 3) ? 255 : 100);
}

void Game::renderPlaceholder(const std::string &title) {
  uint8_t r = m_darkSkin ? 255 : 119;
  uint8_t g = m_darkSkin ? 255 : 110;
  uint8_t b = m_darkSkin ? 255 : 101;

  m_renderer.drawTextCentered(title, m_font, WINDOW_WIDTH / 2, 150, r, g, b,
                              255);
  m_renderer.drawTextCentered("Coming Soon...", m_font, WINDOW_WIDTH / 2, 300,
                              r, g, b, 150);
  m_renderer.drawTextCentered("Press Enter to Return", m_font, WINDOW_WIDTH / 2,
                              450, r, g, b, 255);
}

void Game::renderGameOver() {
  renderPlaying(); // Render the board in background (dimmed?)

  // Overlay
  m_renderer.setDrawColor(255, 255, 255, 150); // Semi-transparent white
  m_renderer.drawFillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  m_renderer.drawTextCentered("GAME OVER", m_font, WINDOW_WIDTH / 2, 150, 119,
                              110, 101, 255);

  uint8_t alpha1 = (m_menuSelection == 0)
                       ? 255
                       : 100; // Selected = Solid, Unselected = Faded
  uint8_t alpha2 = (m_menuSelection == 1) ? 255 : 100;

  m_renderer.drawTextCentered("Restart", m_font, WINDOW_WIDTH / 2, 300, 119,
                              110, 101, alpha1);
  m_renderer.drawTextCentered("Main Menu", m_font, WINDOW_WIDTH / 2, 400, 119,
                              110, 101, alpha2);
}

void Game::renderPlaying() {
  // Render Tiles
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      Core::Tile tile = m_grid.getTile(x, y);
      SDL_Rect rect = getTileRect(x, y);
      Color c = getTileColor(tile.getValue());

      m_renderer.setDrawColor(c.r, c.g, c.b, c.a);
      m_renderer.drawFillRect(rect.x, rect.y, rect.w, rect.h);

      if (!tile.isEmpty()) {
        Color textColor = getTextColor(tile.getValue());
        m_renderer.drawTextCentered(std::to_string(tile.getValue()), m_font,
                                    rect.x + rect.w / 2, rect.y + rect.h / 2,
                                    textColor.r, textColor.g, textColor.b,
                                    textColor.a);
      }
    }
  }

  // Draw Score (Simple HUD)
  std::string scoreText = "Score: " + std::to_string(m_score);
  // Use a contrasting color for text based on theme
  uint8_t r = m_darkSkin ? 255 : 119;
  uint8_t g = m_darkSkin ? 255 : 110;
  uint8_t b = m_darkSkin ? 255 : 101;
  m_renderer.drawText(scoreText, m_font, 20, 10, r, g, b, 255);
}

void Game::resetGame() {
  m_grid = Core::Grid();
  m_grid.spawnRandomTile();
  m_grid.spawnRandomTile();
  m_score = 0;
}

Color Game::getTileColor(int val) const {
  if (m_darkSkin) {
    switch (val) {
    case 0:
      return {50, 50, 50, 255}; // Empty (Dark Grey)
    case 2:
      return {80, 80, 80, 255};
    case 4:
      return {100, 100, 100, 255};
    case 8:
      return {200, 100, 50, 255}; // Orange
    case 16:
      return {220, 120, 50, 255};
    case 32:
      return {240, 140, 50, 255};
    case 64:
      return {255, 100, 0, 255};
    case 128:
      return {255, 200, 0, 255}; // Gold
    case 256:
      return {255, 200, 0, 255};
    case 512:
      return {255, 200, 0, 255};
    case 1024:
      return {255, 200, 0, 255};
    case 2048:
      return {255, 200, 0, 255};
    default:
      return {255, 200, 0, 255}; // Gold
    }
  } else {
    // Classic Light Theme
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
}

Color Game::getTextColor(int val) const {
  if (val <= 4) {
    return {119, 110, 101, 255}; // Dark Text for Light Tiles
  } else {
    return {249, 246, 242, 255}; // White Text for Darker Tiles
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
