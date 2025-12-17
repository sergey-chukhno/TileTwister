#pragma once
#include "InputManager.hpp" // Added
#include "core/GameLogic.hpp"
#include "core/Grid.hpp"
#include "engine/Context.hpp"
#include "engine/Font.hpp"
#include "engine/Renderer.hpp"
#include "engine/Window.hpp"

namespace Game {

enum class GameState {
  MainMenu,
  Playing,
  GameOver,
  Options,
  Leaderboard,
  Achievements,
  LoadGame
};

struct Color {
  uint8_t r, g, b, a;
};

class Game {
public:
  Game();
  ~Game() = default;

  void run();

private:
  void handleInput();
  void update();
  void render();

  // State Handlers
  void handleInputMenu();
  void handleInputPlaying(Action action);
  void handleInputGameOver();
  void handleInputOptions(Action action);
  void
  handleInputPlaceholder(Action action); // For Load/Leaderboard/Achievements

  void renderMenu();
  void renderPlaying();
  void renderGameOver();
  void renderOptions();
  void renderPlaceholder(const std::string &title);

  void resetGame();

  // Scoring
  int m_score;
  int m_bestScore;

  // Rendering Helpers
  [[nodiscard]] Color getTileColor(int value) const;
  [[nodiscard]] SDL_Rect getTileRect(int x, int y) const;

  // Visual Overhaul
  std::unique_ptr<Engine::Texture> m_tileTexture;
  void renderHeader();
  void renderScoreBox(const std::string &label, int value, int x, int y);
  void renderGridBackground();

  [[nodiscard]] Color getBackgroundColor() const;
  [[nodiscard]] Color getGridColor() const;
  [[nodiscard]] Color getEmptyTileColor() const;
  [[nodiscard]] Color getTextColor(int value) const;

  // Engine Components
  Engine::Context m_context;
  Engine::Window m_window;
  Engine::Renderer m_renderer;
  Engine::Font m_font; // Standard Tile Font (Size 40?)

  // Specific Fonts for UI
  Engine::Font m_fontTitle;    // Size 80
  Engine::Font m_fontSmall;    // Size 18 (Labels)
  Engine::Font m_fontMedium;   // Size 30 (Score Values)
  InputManager m_inputManager; // Added

  // Core Components
  Core::Grid m_grid;
  Core::GameLogic m_logic;

  // State
  bool m_isRunning;
  GameState m_state;
  GameState m_previousState; // Added for navigation
  int m_menuSelection;       // Reused for all menus

  // Settings
  bool m_darkSkin; // True = Dark Mode
  bool m_soundOn;  // True = Sound Enabled

  // Constants
  static constexpr int WINDOW_WIDTH = 600;
  static constexpr int WINDOW_HEIGHT = 800;
  static constexpr int TILE_SIZE = 120;
  static constexpr int GRID_PADDING = 20;
  static constexpr int GRID_OFFSET_X = 50;
  static constexpr int GRID_OFFSET_Y = 50;
};

} // namespace Game
