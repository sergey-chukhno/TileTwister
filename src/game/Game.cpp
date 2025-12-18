
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

  // Load Logo
  try {
    m_logoTexture =
        std::make_unique<Engine::Texture>(m_renderer, "assets/logo.png");
  } catch (const std::exception &e) {
    SDL_Log("Failed to load logo: %s", e.what());
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
    handleInputMenu(action, mx, my, clicked);
    break;

  case GameState::Playing:
    handleInputPlaying(action);
    break;

  case GameState::Options:
    handleInputOptions(action, mx, my, clicked);
    break;

  case GameState::Leaderboard:
  case GameState::Achievements:
  case GameState::LoadGame:
    handleInputPlaceholder(action);
    break;

  case GameState::GameOver:
    handleInputGameOver(action, mx, my, clicked);
    break;
  }
}

// --- INPUT HANDLERS ---

void Game::handleInputMenu(Action action, int mx, int my, bool clicked) {
  if (action == Action::Quit) {
    m_isRunning = false;
    return;
  }

  // Layout Constants (Must match renderMenu)
  int cardH = 650;
  int cardY = (WINDOW_HEIGHT - cardH) / 2;
  int startY = cardY + 240;
  int btnH = 50;
  int gap = 20;
  int btnW = 320;
  int btnX = (WINDOW_WIDTH - btnW) / 2;

  // Mouse Over Logic
  int hoverIndex = -1;
  for (int i = 0; i < 6; ++i) {
    int y = startY + i * (btnH + gap);
    if (mx >= btnX && mx <= btnX + btnW && my >= y && my <= y + btnH) {
      hoverIndex = i;
      break;
    }
  }

  if (hoverIndex != -1) {
    m_menuSelection = hoverIndex;
    if (clicked)
      action = Action::Select; // Trigger click
  }

  // Keyboard Navigation
  if (action == Action::Up) {
    m_menuSelection = (m_menuSelection - 1 + 6) % 6;
    m_soundManager.playOneShot("move", 32);
  } else if (action == Action::Down) {
    m_menuSelection = (m_menuSelection + 1) % 6;
    m_soundManager.playOneShot("move", 32);
  } else if (action == Action::Select) {
    m_soundManager.play("move"); // Click sound
    switch (m_menuSelection) {
    case 0:
      m_state = GameState::Playing;
      resetGame();
      break;
    case 1:
      m_state = GameState::LoadGame;
      m_previousState = GameState::MainMenu;
      m_menuSelection = 0;
      break;
    case 2:
      m_state = GameState::Options;
      m_previousState = GameState::MainMenu;
      m_menuSelection = 0;
      break;
    case 3:
      m_state = GameState::Leaderboard;
      m_previousState = GameState::MainMenu;
      m_menuSelection = 0;
      break;
    case 4:
      m_state = GameState::Achievements;
      m_previousState = GameState::MainMenu;
      m_menuSelection = 0;
      break;
    case 5:
      m_isRunning = false;
      break;
    }
  }
}

void Game::handleInputPlaceholder(Action action) {
  if (action == Action::Confirm || action == Action::Back) {
    m_state = GameState::MainMenu;
    m_menuSelection = 0;
  }
}

void Game::handleInputOptions(Action action, int mx, int my, bool clicked) {
  // Mouse Detection
  // Matches renderOptions layout:
  // cardH = 400, cardY = (800-400)/2 = 200
  // startY = 200 + 160 = 360
  // optionW = 350, optionX = (600-350)/2 = 125
  // Item Height ~50 (switches) or check drawSwitch/Button
  // Gap = 70

  // 0: Skin (Y=360 to 410)
  // 1: Sound (Y=430 to 480) -> wait, startY=360 is item 0? No, let's check loop
  // order in render. Render:
  // 1. Sound (m_menuSelection 1). Y = startY = 360.
  // 2. Skin  (m_menuSelection 0). Y = startY + gap = 430.
  // 3. Back  (m_menuSelection 2). Y = cardY + 400 - 80 = 520. Rect: X=200,
  // W=200, H=50.

  // Wait, renderOptions renders Sound as selection 1 and Skin as selection 0?
  // Let's re-verify renderOptions order.
  // DrawSwitch Sound (Sel 1) at startY.
  // DrawSwitch Skin  (Sel 0) at startY + gap.
  // DrawButton Back  (Sel 2) at bottom.
  // This index mapping (1, 0, 2) is confusing but we must match it.

  // Mouse Hover Logic
  int hoverIndex = -1;
  int optionX = 125;
  int optionW = 350;

  // Sound Area (360-410)
  if (mx >= optionX && mx <= optionX + optionW && my >= 360 && my <= 410)
    hoverIndex = 1;

  // Skin Area (430-480)
  if (mx >= optionX && mx <= optionX + optionW && my >= 430 && my <= 480)
    hoverIndex = 0;

  // Back Area (520-570, X=200, W=200)
  if (mx >= 200 && mx <= 400 && my >= 520 && my <= 570)
    hoverIndex = 2;

  if (hoverIndex != -1) {
    m_menuSelection = hoverIndex;
    if (clicked)
      action = Action::Select;
  }

  if (action == Action::Select) {
    switch (m_menuSelection) {
    case 0: // Skin
      m_darkSkin = !m_darkSkin;
      break;
    case 1: // Sound
      m_soundOn = !m_soundOn;
      m_soundManager.toggleMute();
      break;
    case 2: // Back
      m_state = m_previousState;
      // Reset selection when leaving? Maybe not needed as Main Menu resets it.
      break;
    }
  } else if (action == Action::Back) {
    m_state = m_previousState;
  }

  if (action == Action::Up) {
    // Cycle 1 -> 0 -> 2 -> 1 ... based on Y pos?
    // Visual Order: Sound(1), Skin(0), Back(2)
    // If Sel=1 (Sound), Up -> Back(2)
    // If Sel=0 (Skin), Up -> Sound(1)
    // If Sel=2 (Back), Up -> Skin(0)

    if (m_menuSelection == 1)
      m_menuSelection = 2;
    else if (m_menuSelection == 0)
      m_menuSelection = 1;
    else if (m_menuSelection == 2)
      m_menuSelection = 0;

    m_soundManager.playOneShot("move", 32);
  } else if (action == Action::Down) {
    // Visual Order: Sound(1) -> Skin(0) -> Back(2) -> Sound(1)
    if (m_menuSelection == 1)
      m_menuSelection = 0;
    else if (m_menuSelection == 0)
      m_menuSelection = 2;
    else if (m_menuSelection == 2)
      m_menuSelection = 1;

    m_soundManager.playOneShot("move", 32);
  }
}

void Game::handleInputGameOver(Action action, int mx, int my, bool clicked) {
  // Mouse Detection for buttons
  // Try Again (0): Center-Left, Menu (1): Center-Right
  // Layout in renderGameOver:
  // btnW = 160, gap = 20. TotalW = 340. StartX = (W-340)/2 = 130.
  // Y = cardY + 220. cardH=300 => cardY = 250. btnY = 470.
  // Restart: 130 to 290, 470-520.
  // Menu: 310 to 470, 470-520.

  // Note: WINDOW_HEIGHT 800.
  // If we change layout in renderGameOver we must update here.
  // Assuming current renderGameOver constants.

  if (mx > 130 && mx < 470 && my > 470 && my < 520) {
    if (mx < 290)
      m_menuSelection = 0;
    else if (mx > 310)
      m_menuSelection = 1;

    if (clicked)
      action = Action::Select;
  }

  if (action == Action::Select) {
    if (m_menuSelection == 0)
      resetGame();
    else {
      m_state = GameState::MainMenu;
      m_menuSelection = 0;
    }
  } else if (action == Action::Left || action == Action::Right ||
             action == Action::Up || action == Action::Down) {
    m_menuSelection = (m_menuSelection + 1) % 2;
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
  // Render blurred/dimmed grid in background
  renderGridBackground();

  // Overlay Card
  int cardW = 500; // Increased from 400 for better breathing room
  int cardH = 650; // Increased from 500 to fit content comfortably
  int cardX = (WINDOW_WIDTH - cardW) / 2;
  int cardY = (WINDOW_HEIGHT - cardH) / 2;

  drawCard(cardX, cardY, cardW, cardH);

  // Logo
  if (m_logoTexture) {
    // Logo usually 2:1 aspect ratio roughly
    SDL_Rect logoRect = {cardX + 75, cardY + 40, 350, 175};
    m_logoTexture->setColor(255, 255, 255); // Reset color
    m_renderer.drawTexture(*m_logoTexture, logoRect);
  } else {
    m_renderer.drawTextCentered("TILE TWISTER", m_fontTitle, WINDOW_WIDTH / 2,
                                cardY + 80, 119, 110, 101, 255);
  }

  const char *options[] = {"Start Game",  "Load Game",    "Options",
                           "Leaderboard", "Achievements", "Quit"};

  int startY = cardY + 240; // Push buttons down
  int btnW = 320;           // Wider buttons
  int btnH = 50;
  int gap = 20; // More spacing

  for (int i = 0; i < 6; ++i) {
    int btnX = (WINDOW_WIDTH - btnW) / 2;
    int btnY = startY + i * (btnH + gap);

    drawButton(options[i], btnX, btnY, btnW, btnH, (m_menuSelection == i));
  }
}

void Game::renderOptions() {
  // Background
  renderGridBackground();

  // Card Panel
  int cardW = 500;
  int cardH = 400;
  int cardX = (WINDOW_WIDTH - cardW) / 2;
  int cardY = (WINDOW_HEIGHT - cardH) / 2;
  drawCard(cardX, cardY, cardW, cardH);

  // Header
  uint8_t r = m_darkSkin ? 119 : 60;
  uint8_t g = m_darkSkin ? 110 : 60;
  uint8_t b = m_darkSkin ? 101 : 60;
  m_renderer.drawTextCentered("OPTIONS", m_fontTitle, WINDOW_WIDTH / 2,
                              cardY + 70, r, g, b, 255);

  int startY = cardY + 160;
  int optionW = 350;
  int optionX = (WINDOW_WIDTH - optionW) / 2;
  int gap = 70;

  // 1. Sound Toggle
  drawSwitch("Sound", m_soundOn, optionX, startY, optionW,
             (m_menuSelection == 1));

  // 2. Skin Toggle
  drawSwitch(m_darkSkin ? "Dark Mode" : "Light Mode", m_darkSkin, optionX,
             startY + gap, optionW, (m_menuSelection == 0));

  // 3. Back Button
  drawButton("Back", (WINDOW_WIDTH - 200) / 2, cardY + cardH - 80, 200, 50,
             (m_menuSelection == 2));
}

void Game::renderPlaceholder(const std::string &title) {
  renderGridBackground();

  int cardW = 500;
  int cardH = 300;
  int cardX = (WINDOW_WIDTH - cardW) / 2;
  int cardY = (WINDOW_HEIGHT - cardH) / 2;

  drawCard(cardX, cardY, cardW, cardH);

  uint8_t r = m_darkSkin ? 119 : 60;
  uint8_t g = m_darkSkin ? 110 : 60;
  uint8_t b = m_darkSkin ? 101 : 60;

  m_renderer.drawTextCentered(title, m_fontTitle, WINDOW_WIDTH / 2, cardY + 80,
                              r, g, b, 255);

  m_renderer.drawTextCentered("Coming Soon...", m_fontMedium, WINDOW_WIDTH / 2,
                              cardY + 160, r, g, b, 150);

  drawButton("Back", (WINDOW_WIDTH - 200) / 2, cardY + cardH - 80, 200, 50,
             (m_menuSelection == 0));
}

void Game::renderGameOver() {
  // Render Board
  renderPlaying();

  // Overlay Result Card
  int cardW = 400;
  int cardH = 300;
  int cardX = (WINDOW_WIDTH - cardW) / 2;
  int cardY = (WINDOW_HEIGHT - cardH) / 2;

  // Draw Card
  drawCard(cardX, cardY, cardW, cardH);

  // Title
  m_renderer.drawTextCentered("GAME OVER!", m_fontTitle, WINDOW_WIDTH / 2,
                              cardY + 50, 119, 110, 101, 255);

  // Score
  std::string scoreTxt = std::to_string(m_score);
  m_renderer.drawTextCentered(scoreTxt, m_fontTitle, WINDOW_WIDTH / 2,
                              cardY + 120, 119, 110, 101, 255);

  m_renderer.drawTextCentered("Final Score", m_fontSmall, WINDOW_WIDTH / 2,
                              cardY + 175, 119, 110, 101, 180);

  // Buttons: "Try Again" vs "Menu"
  // Horizontal layout
  int btnW = 160;
  int gap = 20;
  int totalBtnW = (btnW * 2) + gap;
  int startX = (WINDOW_WIDTH - totalBtnW) / 2; // Center both buttons
  int btnY = cardY + 220;

  drawButton("Try Again", startX, btnY, btnW, 50, (m_menuSelection == 0));
  drawButton("Menu", startX + btnW + gap, btnY, btnW, 50,
             (m_menuSelection == 1));
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
// --- UI HELPERS ---

void Game::drawCard(int x, int y, int w, int h) {
  // "Glass" background: Dark semi-transparent
  m_renderer.setDrawColor(250, 248, 239, 230); // Light beige glass
  if (m_darkSkin) {
    m_renderer.setDrawColor(30, 30, 30, 230); // Dark glass
  }

  // We don't have rounded rects in SDL_Renderer easily, so just fill rect +
  // border
  m_renderer.drawFillRect(x, y, w, h);

  // Border
  m_renderer.setDrawColor(187, 173, 160, 255);
  // m_renderer.drawRect(x, y, w, h); // SDL_RenderDrawRect
  // Actually Engine::Renderer doesn't expose drawRect yet?
  // Let's stick to fill rect.
}

void Game::drawButton(const std::string &text, int x, int y, int w, int h,
                      bool selected) {
  SDL_Rect rect = {x, y, w, h};

  // Hover/Selected Animation Effect
  if (selected) {
    // Enlarge slightly
    int growth = 4;
    rect.x -= growth / 2;
    rect.y -= growth / 2;
    rect.w += growth;
    rect.h += growth;
  }

  // Background Color
  // Selected: Orange (#f67c5f), Normal: Brown (#8f7a66)
  Color btnColor =
      selected ? Color{246, 124, 95, 255} : Color{143, 122, 102, 255};

  if (m_tileTexture) {
    m_tileTexture->setColor(btnColor.r, btnColor.g, btnColor.b);
    m_renderer.drawTexture(*m_tileTexture, rect);
  } else {
    m_renderer.setDrawColor(btnColor.r, btnColor.g, btnColor.b, 255);
    m_renderer.drawFillRect(rect.x, rect.y, rect.w, rect.h);
  }

  // Text
  m_renderer.drawTextCentered(text, m_fontMedium, rect.x + rect.w / 2,
                              rect.y + rect.h / 2 - 2, 255, 255, 255, 255);
}

void Game::drawSwitch(const std::string &label, bool value, int x, int y, int w,
                      bool selected) {
  // Label
  uint8_t r = m_darkSkin ? 249 : 119;
  uint8_t g = m_darkSkin ? 246 : 110;
  uint8_t b = m_darkSkin ? 242 : 101;

  m_renderer.drawText(label, m_fontMedium, x, y, r, g, b, selected ? 255 : 150);

  // Switch Graphic (Right Aligned in width w)
  int switchW = 60;
  int switchH = 30;
  int switchX = x + w - switchW;
  int switchY = y;

  // Background (Track)
  Color trackColor = value ? Color{246, 124, 95, 255}
                           : Color{200, 200, 200, 255}; // Orange or Grey

  // Draw rounded rect using tile texture if possible
  SDL_Rect trackRect = {switchX, switchY, switchW, switchH};
  if (m_tileTexture) {
    m_tileTexture->setColor(trackColor.r, trackColor.g, trackColor.b);
    m_renderer.drawTexture(*m_tileTexture, trackRect);
  } else {
    m_renderer.setDrawColor(trackColor.r, trackColor.g, trackColor.b, 255);
    m_renderer.drawFillRect(switchX, switchY, switchW, switchH);
  }

  // Knob
  int knobSize = 26;
  int knobX = value ? (switchX + switchW - knobSize - 2) : (switchX + 2);
  int knobY = switchY + 2;

  m_renderer.setDrawColor(255, 255, 255, 255);
  m_renderer.drawFillRect(knobX, knobY, knobSize, knobSize);
}

// --- RENDER HELPERS ---

void Game::renderGridBackground() {
  Color gridColor = getGridColor();

  // Layout V2: Y=180, Size=450
  int gridY = 180;
  int gridSize = 450;
  int marginX = (WINDOW_WIDTH - gridSize) / 2;

  // Check if tile texture is loaded to use for background tiles
  // Actually, we removed the full background and used transparent board.
  // But for the menus, we want to see the TILES in the background if game is
  // playing? Or just empty grid? Let's render the Playing state logic for tiles
  // but dimmed? Re-using renderPlaying() is tricky if we want blur. Let's just
  // render the static empty grid slots for visual context.

  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      SDL_Rect rect = getTileRect(x, y);
      Color c = getEmptyTileColor();

      if (m_tileTexture) {
        m_tileTexture->setColor(c.r, c.g, c.b);
        m_renderer.drawTexture(*m_tileTexture, rect);
      } else {
        m_renderer.setDrawColor(c.r, c.g, c.b, 255);
        m_renderer.drawFillRect(rect.x, rect.y, rect.w, rect.h);
      }
    }
  }
}

// Helpers
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
