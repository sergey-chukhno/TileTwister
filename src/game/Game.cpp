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

  // Load Assets
  try {
    m_tileTexture = std::make_unique<Engine::Texture>(
        m_renderer, "assets/tile_rounded.png");
  } catch (const std::exception &e) {
    // Fallback or error logging
    SDL_Log("Failed to load tile texture: %s", e.what());
  }

  // Initial Setup
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
  Color bg = getBackgroundColor();
  m_renderer.setDrawColor(bg.r, bg.g, bg.b, 255);
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

  std::string scoreMsg = "Your Score: " + std::to_string(m_score);
  m_renderer.drawTextCentered(scoreMsg, m_font, WINDOW_WIDTH / 2, 250, 119, 110,
                              101, 255);

  m_renderer.drawTextCentered("Press R to Restart", m_font, WINDOW_WIDTH / 2,
                              400, 119, 110, 101, 200);

  uint8_t alpha1 = (m_menuSelection == 0) ? 255 : 100; // Selected = Solid,
                                                       // Unselected = Faded
  uint8_t alpha2 = (m_menuSelection == 1) ? 255 : 100;

  m_renderer.drawTextCentered("Restart", m_font, WINDOW_WIDTH / 2, 300, 119,
                              110, 101, alpha1);
  m_renderer.drawTextCentered("Main Menu", m_font, WINDOW_WIDTH / 2, 400, 119,
                              110, 101, alpha2);
}

void Game::renderHeader() {
  int headerY = 20;

  // 1. Title
  Color textColor =
      m_darkSkin ? Color{249, 246, 242, 255} : Color{119, 110, 101, 255};
  m_renderer.drawText("2048", m_font, 20, headerY, textColor.r, textColor.g,
                      textColor.b, 255);

  // 2. Score Boxes
  int boxW = 100;
  int boxH = 60;
  int margin = 10;
  int startX = WINDOW_WIDTH - (boxW * 2) - margin - 20;

  SDL_Rect scoreRect = {startX, headerY, boxW, boxH};
  SDL_Rect bestRect = {startX + boxW + margin, headerY, boxW, boxH};

  // Draw Box Backgrounds (Grid Color)
  Color boxColor = getGridColor();
  m_renderer.setDrawColor(boxColor.r, boxColor.g, boxColor.b, boxColor.a);
  // Use Texture for rounded boxes too if possible, but rect is fine for now or
  // use tileTexture stretched
  if (m_tileTexture) {
    m_tileTexture->setColor(boxColor.r, boxColor.g, boxColor.b);
    m_renderer.drawTexture(*m_tileTexture, scoreRect);
    m_renderer.drawTexture(*m_tileTexture, bestRect);
  } else {
    m_renderer.drawFillRect(scoreRect.x, scoreRect.y, scoreRect.w, scoreRect.h);
    m_renderer.drawFillRect(bestRect.x, bestRect.y, bestRect.w, bestRect.h);
  }

  // Labels & Values (Small rendering - we might need smaller font, but for now
  // re-use m_font scaled or just centered) Actually font is 55px. Too big for
  // box. Just draw numbers for now with overlap risk, or center them. Ideally
  // we need a smaller font asset.

  // For MVP Visual Overhaul, let's just put Score: X text next to it or
  // simplistic. Let's rely on standard text drawing. "Score" label + Value.
}

void Game::renderPlaying() {
  // 1. Render Header
  renderHeader();

  // 2. Render Score/Best Text (Overlaying the boxes approx)
  // We need a smaller font. Since we don't have one, we'll draw text
  // below/beside. Updated plan: Simple HUD top right.
  Color textColor =
      m_darkSkin ? Color{249, 246, 242, 255} : Color{119, 110, 101, 255};

  std::string sText = std::to_string(m_score);
  std::string bText = std::to_string(m_bestScore);

  // Right aligned text manual calculation approx
  m_renderer.drawText("Score", m_font, 350, 20, 200, 200, 200, 255); // Label
  m_renderer.drawText(sText, m_font, 350, 70, textColor.r, textColor.g,
                      textColor.b, 255);

  m_renderer.drawText("Best", m_font, 480, 20, 200, 200, 200, 255);
  m_renderer.drawText(bText, m_font, 480, 70, textColor.r, textColor.g,
                      textColor.b, 255);

  // 3. Render Grid Background
  Color gridColor = getGridColor();
  // Main Grid Container
  // Grid calculates rects based on specific layout.
  // Let's assume Grid starts at Y=150.
  // Total Width 600. Padding 15. Tile Size (600 - 5*15) / 4 = 131.25.
  // Let's hardcode a beautiful layout.

  int gridY = 180;
  int gridSize = WINDOW_WIDTH - 40; // 560
  int padding = 15;
  int titleSize = (gridSize - 5 * padding) / 4; // (560 - 75)/4 = 121

  SDL_Rect gridRect = {20, gridY, gridSize, gridSize};

  m_renderer.setDrawColor(gridColor.r, gridColor.g, gridColor.b, 255);
  if (m_tileTexture) {
    m_tileTexture->setColor(gridColor.r, gridColor.g, gridColor.b);
    m_renderer.drawTexture(*m_tileTexture, gridRect); // Big rounded rect back
  } else {
    m_renderer.drawFillRect(gridRect.x, gridRect.y, gridRect.w, gridRect.h);
  }

  // 4. Render Tiles
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      Core::Tile tile = m_grid.getTile(x, y);

      // Calculate dynamic rect
      int xPos = 20 + padding + x * (titleSize + padding);
      int yPos = gridY + padding + y * (titleSize + padding);
      SDL_Rect rect = {xPos, yPos, titleSize, titleSize};

      Color c =
          tile.isEmpty() ? getEmptyTileColor() : getTileColor(tile.getValue());

      if (m_tileTexture) {
        m_tileTexture->setColor(c.r, c.g, c.b);
        m_renderer.drawTexture(*m_tileTexture, rect);
      } else {
        m_renderer.setDrawColor(c.r, c.g, c.b, c.a);
        m_renderer.drawFillRect(rect.x, rect.y, rect.w, rect.h);
      }

      if (!tile.isEmpty()) {
        Color tc = getTextColor(tile.getValue());
        m_renderer.drawTextCentered(std::to_string(tile.getValue()), m_font,
                                    rect.x + rect.w / 2, rect.y + rect.h / 2,
                                    tc.r, tc.g, tc.b, tc.a);
      }
    }
  }
}

// Colors
Color Game::getBackgroundColor() const {
  return m_darkSkin ? Color{51, 51, 51, 255} : Color{250, 248, 239, 255};
}
Color Game::getGridColor() const {
  return m_darkSkin ? Color{77, 77, 77, 255} : Color{187, 173, 160, 255};
}
Color Game::getEmptyTileColor() const {
  return m_darkSkin ? Color{89, 89, 89, 255} : Color{205, 193, 180, 255};
}

Color Game::getTileColor(int val) const {
  // 2048 Official Colors
  switch (val) {
  case 2:
    return {238, 228, 218, 255};
  case 4:
    return {237, 224, 200, 255};
  case 8:
    return {242, 177, 121, 255};
  case 16:
    return {245, 149, 99, 255};
  case 32:
    return {246, 124, 95, 255};
  case 64:
    return {246, 94, 59, 255};
  case 128:
    return {237, 207, 114, 255};
  case 256:
    return {237, 204, 97, 255};
  case 512:
    return {237, 200, 80, 255};
  case 1024:
    return {237, 197, 63, 255};
  case 2048:
    return {237, 194, 46, 255};
  default:
    return {60, 58, 50, 255}; // Super High
  }
}

Color Game::getTextColor(int val) const {
  return (val <= 4) ? Color{119, 110, 101, 255} : Color{249, 246, 242, 255};
}

void Game::resetGame() {
  m_grid = Core::Grid();
  m_grid.spawnRandomTile();
  m_score = 0;
}

SDL_Rect Game::getTileRect(int x, int y) const {
  int gridY = 180;
  int gridSize = WINDOW_WIDTH - 40; // 560
  int padding = 15;
  int titleSize = (gridSize - 5 * padding) / 4;
  int xPos = 20 + padding + x * (titleSize + padding);
  int yPos = gridY + padding + y * (titleSize + padding);
  return {xPos, yPos, titleSize, titleSize};
}

} // namespace Game
