
#include "Game.hpp"
#include <SDL.h>
#include <cmath>
#include <iostream>
#include <string>

namespace Game {

Game::Game()
    : m_context(), m_window("Tile Twister - 2048", WINDOW_WIDTH, WINDOW_HEIGHT),
      m_renderer(m_window, WINDOW_WIDTH, WINDOW_HEIGHT),
      m_font("assets/ClearSans-Bold.ttf", 40),       // Tile Font
      m_fontTitle("assets/ClearSans-Bold.ttf", 80),  // Title
      m_fontSmall("assets/ClearSans-Bold.ttf", 16),  // Labels
      m_fontMedium("assets/ClearSans-Bold.ttf", 30), // Score Values
      m_inputManager(), m_grid(), m_logic(), m_isRunning(true),
      m_state(GameState::MainMenu), m_previousState(GameState::MainMenu),
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
  if (m_soundManager.init()) {
    m_soundManager.loadSound("move", "assets/move.wav");
    m_soundManager.loadSound("merge", "assets/merge.wav");
    m_soundManager.loadSound("spawn", "assets/spawn.wav");
    m_soundManager.loadSound("invalid", "assets/invalid.wav");
    m_soundManager.loadSound("gameover", "assets/gameover.wav");
    m_soundManager.loadSound("score", "assets/score.wav");
  }

  resetGame();
}

void Game::run() {
  std::cout << "Game Loop Started." << std::endl;

  Uint64 lastTime = SDL_GetTicks64();

  while (m_isRunning) {
    Uint64 currentTime = SDL_GetTicks64();
    float dt = (currentTime - lastTime); // ms
    lastTime = currentTime;

    handleInput();
    update(dt);
    render();

    // Cap at ~60 FPS (Optional, vsync is better but simple delay works)
    if (dt < 16) {
      SDL_Delay(16 - dt);
    }
  }

  std::cout << "Game Loop Ended." << std::endl;
}

void Game::handleInput() {
  int mx = 0, my = 0;
  bool clicked = false;
  Action action = m_inputManager.pollAction(mx, my, clicked);

  if (action == Action::Quit) {
    m_isRunning = false;
    return;
  }

  // Specific Handling for Playing State Buttons (Global check simplifies
  // things if state matches)
  if (m_state == GameState::Playing && clicked) {
    // Toolbar Button Detection (Enlarged Hitboxes)
    // Restart: X=20, Y=120, W=130, H=40
    if (mx >= 20 && mx <= 150 && my >= 120 && my <= 160) {
      resetGame();
      return;
    }
    // Options: X=460, Y=120, W=130, H=40
    if (mx >= 460 && mx <= 590 && my >= 120 && my <= 160) {
      m_previousState = GameState::Playing; // Track history
      m_state = GameState::Options;
      return;
    }
  }

  // Regular Action Handling
  // If Animating, block Input (except Quit?)
  if (m_state == GameState::Animating)
    return;

  switch (m_state) {
  case GameState::Animating:
    break; // Handled above, but satisfies switch
  case GameState::MainMenu:
    // Menu needs Action::Up/Down, does not use mouse yet
    if (action == Action::Up) {
      m_menuSelection = (m_menuSelection - 1 + 6) % 6;
    } else if (action == Action::Down) {
      m_menuSelection = (m_menuSelection + 1) % 6;
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
      case 2:                                  // Options
        m_previousState = GameState::MainMenu; // Track history
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
    // Also check mouse clicks for buttons
    if (clicked) {
      // Restart Button Y=320, Main Menu Y=390. Center X=300
      // Approx detection
      if (mx > 200 && mx < 400) {
        if (my > 300 && my < 340) { // Restart area
          resetGame();
          m_state = GameState::Playing;
        }
        if (my > 370 && my < 410) { // Main Menu area
          m_state = GameState::MainMenu;
          m_menuSelection = 0;
        }
      }
    }
    break;
  }
}

void Game::handleInputOptions(Action action) {
  if (action == Action::Up) {
    m_menuSelection--;
    if (m_menuSelection < 0)
      m_menuSelection = 3; // 3 Options (Theme, Sound, Controls, Back) ->
                           // Wait, 4 items
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
      m_soundManager.toggleMute();
      break;
    case 2:
      break; // Controls is just info
    case 3:  // Back
      m_state = m_previousState;
      m_menuSelection = 2; // Return to "Options" highlighted
      break;
    }
  } else if (action == Action::Back) {
    m_state = m_previousState;
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
  if (action == Action::None || action == Action::Quit ||
      action == Action::Restart || action == Action::Confirm)
    return;

  Core::Direction dir;
  if (action == Action::Up)
    dir = Core::Direction::Up;
  else if (action == Action::Down)
    dir = Core::Direction::Down;
  else if (action == Action::Left)
    dir = Core::Direction::Left;
  else if (action == Action::Right)
    dir = Core::Direction::Right;
  else
    return;

  // Execute Logic with MoveEvents
  // Execute Logic with MoveEvents
  auto result = m_logic.move(m_grid, dir);

  if (result.moved) {
    m_score += result.score;
    if (m_score > m_bestScore)
      m_bestScore = m_score;

    // Process Events for Animation
    bool hasAnimations = false;
    for (const auto &evt : result.events) {
      SDL_Rect fromRect = getTileRect(evt.fromX, evt.fromY); // Pixel Coords
      SDL_Rect toRect = getTileRect(evt.toX, evt.toY);

      Animation anim;
      anim.duration = 0.15f; // Slide duration in seconds

      if (evt.type == Core::GameLogic::MoveEvent::Type::Slide ||
          evt.type == Core::GameLogic::MoveEvent::Type::Merge) {

        // Sound: Slide (One Shot per frame)
        m_soundManager.playOneShot("move", 64);

        anim.type = Animation::Type::Slide;
        anim.value = evt.value;
        anim.startX = (float)fromRect.x;
        anim.startY = (float)fromRect.y;
        anim.endX = (float)toRect.x;
        anim.endY = (float)toRect.y;
        anim.startScale = 1.0f;
        anim.endScale = 1.0f;

        m_animationManager.addAnimation(anim);

        // Hide destination until animation arrives
        m_hiddenTiles.insert({evt.toX, evt.toY});

        if (evt.type == Core::GameLogic::MoveEvent::Type::Merge) {
          // Sound: Merge (Allow overlap)
          m_soundManager.play("merge");

          // Visual: Score Popup
          Animation scoreAnim;
          scoreAnim.type = Animation::Type::Score;
          // Center score on the MERGE destination (toRect/endX,endY)
          scoreAnim.startX = (float)toRect.x + (toRect.w / 2.0f);
          scoreAnim.startY = (float)toRect.y;
          scoreAnim.duration = 0.8f;
          scoreAnim.text = "+" + std::to_string(evt.value);

          // Dynamic Color based on tile value
          Color c = getTileColor(evt.value);
          // Use the tile color, but maybe safeguard alpha?
          scoreAnim.color = {c.r, c.g, c.b, 255};

          m_animationManager.addAnimation(scoreAnim);
          m_soundManager.play("score", 64);
        }

        hasAnimations = true;
      }
    }

    // SPAWN NEW TILE
    auto [sx, sy] = m_grid.spawnRandomTile();
    if (sx != -1) {
      // Sound: Spawn
      m_soundManager.play("spawn");

      SDL_Rect sRect = getTileRect(sx, sy);
      Animation spawnAnim;
      spawnAnim.type = Animation::Type::Spawn;
      spawnAnim.value = m_grid.getTile(sx, sy).getValue();
      spawnAnim.startX = (float)sRect.x;
      spawnAnim.startY = (float)sRect.y;
      spawnAnim.endX = (float)sRect.x;
      spawnAnim.endY = (float)sRect.y; // Static pos
      spawnAnim.startScale = 0.0f;
      spawnAnim.endScale = 1.0f;
      spawnAnim.duration = 0.12f; // 120ms

      m_animationManager.addAnimation(spawnAnim);
      m_hiddenTiles.insert({sx, sy});
      hasAnimations = true;
    }

    if (hasAnimations) {
      m_state = GameState::Animating;
    } else {
      if (m_logic.isGameOver(m_grid)) {
        m_state = GameState::GameOver;
        m_soundManager.play("gameover");
        m_menuSelection = 0;
      }
    }
  } else {
    // Invalid Move -> Shake
    m_soundManager.playOneShot("invalid");

    Animation shakeAnim;
    shakeAnim.type = Animation::Type::Shake;
    shakeAnim.duration = 0.3f;
    shakeAnim.shakeOffsetX = 10.0f; // 10px shake magnitude

    m_animationManager.addAnimation(shakeAnim);
    m_state = GameState::Animating; // Block input while shaking
  }
}

void Game::update(float dt) { // Added dt
  m_soundManager.update();    // Reset One-Shot flags

  // Animation State Handling
  m_animationManager.update(
      dt /
      1000.0f); // Always update animations (even non-blocking ones like Score)

  if (m_state == GameState::Animating) {
    if (!m_animationManager.hasBlockingAnimations()) {
      m_state = GameState::Playing;
      m_hiddenTiles
          .clear(); // Show static tiles once blocking animations are done

      // Post-Move Check: Game Over?
      if (m_logic.isGameOver(m_grid)) {
        m_state = GameState::GameOver;
        m_menuSelection = 0; // Reset selection for Game Over menu
      }

      // Spawn new tile if we haven't already in logic?
      // Logic::move DOES NOT SPAWN. We need to spawn manually after move.
      // BUT move() in logic returned result.
      // We should spawn and animate the spawn immediately?
      // YES.
      // Strategy:
      // 1. handleInput calls logic.move.
      // 2. logic.move returns events (Slides/Merges).
      // 3. We queue Slide Animations.
      // 4. We ALSO call grid.spawnRandomTile().
      // 5. We queue Spawn Animation for that new tile.
    }
  }

  switch (m_state) {
  case GameState::Animating:
    break; // Logic handled in if block above
  default:
    break;
  }
}

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
  case GameState::Animating: // Render playing state even when animating
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

  // GAME OVER (Y=150)
  m_renderer.drawTextCentered("GAME OVER", m_font, WINDOW_WIDTH / 2, 150, 119,
                              110, 101, 255);

  // Score (Y=230)
  std::string scoreMsg = "Your Score: " + std::to_string(m_score);
  m_renderer.drawTextCentered(scoreMsg, m_font, WINDOW_WIDTH / 2, 230, 119, 110,
                              101, 255);

  // Buttons (Removing overlapping instructions)
  uint8_t alpha1 = (m_menuSelection == 0) ? 255 : 100;
  uint8_t alpha2 = (m_menuSelection == 1) ? 255 : 100;

  // Restart (Y=320)
  m_renderer.drawTextCentered("Restart", m_font, WINDOW_WIDTH / 2, 320, 119,
                              110, 101, alpha1);

  // Main Menu (Y=390)
  m_renderer.drawTextCentered("Main Menu", m_font, WINDOW_WIDTH / 2, 390, 119,
                              110, 101, alpha2);
}

void Game::renderScoreBox(const std::string &label, int value, int x, int y) {
  int boxW = 80; // Compact
  int boxH = 55;

  // Background
  SDL_Rect rect = {x, y, boxW, boxH};
  Color boxColor = {187, 173, 160, 255}; // #bbada0
  m_renderer.setDrawColor(boxColor.r, boxColor.g, boxColor.b, 255);

  if (m_tileTexture) {
    m_tileTexture->setColor(boxColor.r, boxColor.g, boxColor.b);
    m_renderer.drawTexture(*m_tileTexture, rect);
  } else {
    m_renderer.drawFillRect(rect.x, rect.y, rect.w, rect.h);
  }

  // Label "SCORE" or "BEST"
  Color labelColor = {238, 228, 218, 255}; // #eee4da
  m_renderer.drawTextCentered(label, m_fontSmall, x + boxW / 2, y + 15,
                              labelColor.r, labelColor.g, labelColor.b, 255);

  // Value
  Color valueColor = {255, 255, 255, 255};
  m_renderer.drawTextCentered(std::to_string(value), m_fontMedium, x + boxW / 2,
                              y + 38, valueColor.r, valueColor.g, valueColor.b,
                              255);
}

void Game::renderHeader() {
  int headerY = 30; // 20->30 for spacing

  // 1. Title "2048"
  Color titleColor = {119, 110, 101, 255}; // #776e65
  // Adjusted position
  m_renderer.drawText("2048", m_fontTitle, 20, headerY - 10, titleColor.r,
                      titleColor.g, titleColor.b, 255);

  // 2. Score Boxes
  // Align Right
  int boxW = 80;
  int margin = 10;
  int startX = WINDOW_WIDTH - (boxW * 2) - margin - 20;

  renderScoreBox("SCORE", m_score, startX, headerY);
  renderScoreBox("BEST", m_bestScore, startX + boxW + margin, headerY);

  // Subtext: "Join the numbers and get to the 2048 tile!" (Optional, maybe
  // later)
}

void Game::renderPlaying() {
  // 1. Render Header (includes HUD)
  renderHeader();

  // 1.5 Render Toolbar (Restart / Options)
  int toolbarY = 120;
  Color btnColor = {119, 110, 101, 255}; // #776e65
  // Used m_fontMedium (Size 30) for prominence
  m_renderer.drawText("Restart", m_fontMedium, 20, toolbarY + 5, btnColor.r,
                      btnColor.g, btnColor.b, 255);
  m_renderer.drawText("Options", m_fontMedium, 460, toolbarY + 5, btnColor.r,
                      btnColor.g, btnColor.b, 255);

  // Rounded Box for buttons visual cue? (Optional)
  // SDL_Rect rBtn = {20, toolbarY, 100, 40};
  // SDL_Rect oBtn = {480, toolbarY, 100, 40};
  // m_renderer.setDrawColor(187, 173, 160, 255); // Bg color
  // But strictly standard 2048 usually has text buttons or smaller boxes.
  // Text is fine for "Clean" look.

  // --- Calculate Shake Offset ---
  int shakeX = 0;
  for (const auto &anim : m_animationManager.getAnimations()) {
    if (anim.type == Animation::Type::Shake) {
      float t = anim.getProgress();
      float decay = 1.0f - t;
      // Simple sine shake: 3 cycles * decay
      shakeX =
          static_cast<int>(std::sin(t * 20.0f) * anim.shakeOffsetX * decay);
    }
  }

  // 2. Render Grid Background
  Color gridColor = getGridColor();

  // Layout V2: Y=180, Size=450
  int gridY = 180;
  int gridSize = 450;
  int marginX = (WINDOW_WIDTH - gridSize) / 2; // (600-450)/2 = 75

  // Background Removed (Transparent Board) but logic remains here

  // 3. Render Tiles
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      // SKIP rendering if this tile is currently being animated (target of
      // animation) Note: We hide the TARGET of the slide.
      if (m_hiddenTiles.count({x, y}))
        continue;

      Core::Tile tile = m_grid.getTile(x, y);

      SDL_Rect rect = getTileRect(x, y); // Use the helper
      rect.x += shakeX;                  // Apply Shake

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

  // 4. Render Animations (Slide/Spawn/Merge/Score)
  for (const auto &anim : m_animationManager.getAnimations()) {
    if (anim.type == Animation::Type::Shake)
      continue; // Skip shake logic

    // Float/Score Logic
    if (anim.type == Animation::Type::Score) {
      float t = anim.getProgress();
      // Float Up by 50px
      float curY = anim.startY - (50.0f * t);
      float curX = anim.startX;

      // Center on the tile position provided (assume startX/Y are Tile Coords
      // not Pixels?) Wait, handleInput puts Pixel coords or Tile coords?
      // Usually we want Pixel coords. We will handle that in handleInput.
      // Assuming input is Pixel start coords.

      SDL_Rect rect = {
          (int)curX + shakeX, (int)curY, 0,
          0}; // Apply Shake? Maybe not for float text to keep it decoupled?
      // If board shakes, scores usually shake too.

      uint8_t alpha = static_cast<uint8_t>(255 * (1.0f - t));

      // Render Text
      m_renderer.drawTextCentered(anim.text, m_fontMedium, rect.x, rect.y,
                                  anim.color.r, anim.color.g, anim.color.b,
                                  alpha);
      continue;
    }

    // Slide/Spawn Logic
    float t = anim.getProgress();
    float curX = anim.startX + (anim.endX - anim.startX) * t;
    float curY = anim.startY + (anim.endY - anim.startY) * t;

    // Scaling for Spawn
    float curScale = anim.startScale + (anim.endScale - anim.startScale) * t;

    SDL_Rect r;
    SDL_Rect sz = getTileRect(0, 0); // Base size
    int w = static_cast<int>(sz.w * curScale);
    int h = static_cast<int>(sz.h * curScale);

    // Center the scaled rect
    int finalX = (int)curX + (sz.w - w) / 2 + shakeX; // Apply Shake
    int finalY = (int)curY + (sz.h - h) / 2;

    r.x = finalX;
    r.y = finalY;
    r.w = w;
    r.h = h;

    // Render
    Color c = getTileColor(anim.value);

    // Alpha for Merge?
    // Just solid for now.

    if (m_tileTexture) {
      m_tileTexture->setColor(c.r, c.g, c.b);
      m_renderer.drawTexture(*m_tileTexture, r);
    } else {
      m_renderer.setDrawColor(c.r, c.g, c.b, 255);
      m_renderer.drawFillRect(r.x, r.y, r.w, r.h);
    }

    // Text
    Color tc = getTextColor(anim.value);
    m_renderer.drawTextCentered(std::to_string(anim.value), m_font,
                                r.x + r.w / 2, r.y + r.h / 2, tc.r, tc.g, tc.b,
                                255);
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
  if (m_darkSkin) {
    // Neon Palette
    switch (val) {
    case 2:
      return {34, 181, 255, 255}; // Bright Blue
    case 4:
      return {0, 133, 255, 255}; // Deep Blue
    case 8:
      return {255, 206, 0, 255}; // Yellow
    case 16:
      return {255, 153, 0, 255}; // Orange
    case 32:
      return {255, 85, 0, 255}; // Red-Orange
    case 64:
      return {255, 0, 68, 255}; // Red/Pink
    case 128:
      return {0, 255, 204, 255}; // Cyan/Mint
    case 256:
      return {0, 255, 136, 255}; // Green
    case 512:
      return {0, 255, 0, 255}; // Lime
    default:
      return {255, 255, 255, 255}; // White (Super)
    }
  } else {
    // Classic Light Palette
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
      return {60, 58, 50, 255};
    }
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
  int gridSize = 450;
  int marginX = (WINDOW_WIDTH - gridSize) / 2; // 75
  int padding = 15;
  int titleSize = (gridSize - 5 * padding) / 4;
  int xPos = marginX + padding + x * (titleSize + padding);
  int yPos = gridY + padding + y * (titleSize + padding);
  return {xPos, yPos, titleSize, titleSize};
}

} // namespace Game
