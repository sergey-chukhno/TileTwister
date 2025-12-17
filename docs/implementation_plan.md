# Implementation Plan - Tile Twister

# Goal
Build a C++2048 clone using modern C++17/20 and SDL2, focusing on clean architecture and testability.

## User Review Required
> [!NOTE]
> **Portability strategy**: We will use CMake + a package manager (CMake's `FetchContent`) to handle SDL2 and GTest. This ensures your Windows teammates can just run `camke` and it works, without needing Docker containers which struggle with GUI apps.

## Proposed Milestones

### Milestone 1: Project Skeleton & Build System
**Goal**: A compiling "Hello World" that links GTest and SDL2.
- Setup `CMakeLists.txt`
- Configure `GoogleTest`
- Configure `SDL2`
- Verify compiling on Mac (User) and ensure CMake scripts## Phase E: Architecture Refactoring & Cleanup
Currently, `Game` class does too much (God Object). We need to split it.
### 1. Input Management
*   Extract `InputManager` class.
*   Maps raw SDL scancodes (e.g., `SDLK_UP`) to logical Game Actions (`Action::MoveUp`).
*   **Why?** Allows remapping keys or adding Controller support later easily.

### 2. State Management
*   The game currently just "runs". It needs states: `Menu`, `Playing`, `GameOver`.
*   Implement a simple `GameState` enum or State Pattern.

## Detailed Design: Phase E (Refactoring)

### 1. InputManager (The Translator)
**Problem**: `Game.cpp` currently has raw SDL event loops and `SDLK_UP` hardcoded.
**Solution**: Create an abstract "Action" layer.
*   **Enum**: `enum class Action { None, Up, Down, Left, Right, Quit, Restart, Confirm };`
*   **Class**: `InputManager`
    *   `Action pollAction()`: Polls SDL events and returns a high-level `Action`.
    *   **Benefit**: If we want to add WASD support, we just change `InputManager`. The `Game` class doesn't care.

### 2. Game State (The Flow)
**Problem**: The game is always "Playing". We need menus and game over screens.
**Solution**: Finite State Machine (FSM).
*   **Enum**: `enum class GameState { MainMenu, Playing, GameOver };`
*   **Loop**:
    ```cpp
    void Game::update() {
        switch (m_state) {
            case GameState::MainMenu: updateMenu(); break;
            case GameState::Playing:  updateGame(); break;
            case GameState::GameOver: updateGameOver(); break;
        }
    }
    ```
*   **Why**: Strictly separates logic. You can't "move a tile" while in the Menu.

## Phase F: Complete Gameplay Loop
### 1. Scoring System
*   **Core**: Update `GameLogic::slideAndMergeRow` to return points earned.
*   **Game**: Track `currentScore` and `bestScore`.
*   **UI**: Render Score at the top.

### 2. Game Over State
*   **Logic**: Implement `GameLogic::isGameOver(grid)` (checking full grid + no possible moves).
*   **UI**: Detect Game Over -> Stop input -> Show "Game Over" text -> Press 'R' to restart.

## Phase G: Integration Testing
We have Unit Tests (GTest) for `Core`. We need Integration Tests for `Game`.
*   **Headless Game Loop**: Create a `HeadlessGame` that runs `GameLogic` without `Renderer`.
*   **Scripted Scenarios**: Feed a list of inputs (Up, Up, Left...) and verify the final Grid state matches expected output.
*   **Why?** Ensures the "Controller" (Game class) correctly wires inputs to logic.

## Phase H: "Aller plus loin" (Advanced Features)
*   **Undo/Redo**: Store a `std::stack<Grid>` of history.
*   **Save/Load**: Serialize `Grid` and `Score` to a file (`savegame.dat`).
*   **AI Solver**: Implement a simple Expectimax or Monte Carlo solver to play the game automatically.
*   **Animations**: Smooth sliding transitions (requires changing Rendering to be time-based).
- **Files**:
    - `src/game/Game.hpp/cpp` (Main loop, Event polling)
    - `src/game/InputHandler.hpp` (Map keys to Directions)
    - `main.cpp`
- **Verification**: Playable game.

### Milestone 2: Core Game Logic (The Model)
**Goal**: Fully testable 2048 logic without any graphics.
- **Files**:
    - `src/core/Tile.hpp/cpp` (Value, empty state, merging flag)
    - `src/core/Grid.hpp/cpp` (4x4 matrix, spawning logic)
    - `src/core/GameLogic.hpp/cpp` (Move validation, sliding, merging rules)
- **Verification**: Unit tests covering all movement and merge cases (e.g., `[2, 2, 0, 0] -> Left -> [4, 0, 0, 0]`).

### Milestone 3: The Engine (The View)
**Goal**: A window that can render rectangles and respond to quit events.
- **Files**:
    - `src/engine/Window.hpp/cpp` (RAII wrapper for `SDL_Window`)
    - `src/engine/Renderer.hpp/cpp` (RAII wrapper for `SDL_Renderer`)
    - `src/engine/Color.hpp`
- **Verification**: Visual check (Window opens, closes cleanly).

### Milestone 4: Integration (The Controller)
**Goal**: Connect Input -> Logic -> Rendering.
- **Files**:
    - `src/game/Game.hpp/cpp` (Main loop, Event polling)
    - `src/game/InputHandler.hpp` (Map keys to Directions)
    - `main.cpp`
- **Verification**: Playable game.

### Milestone 5: "Aller plus loin" (Bonus Features)
**Goal**: Polish and Gameplay Enhancements.
- New Objectives (reach 1024, 8192).
- Save/Load Game state.
- Special Tiles (blocked, bonus).
- Animations (Start with simple interpolation).

### Milestone 6: High-Performance Optimization & AI
**Goal**: Advanced C++ Engineering & Benchmarking.
- **Bitboard Implementation**: Re-implement `Grid` using `uint64_t` and bitwise operations (`<<`, `&`, `|`).
- **Benchmarking**: Create a performance showdown: `ArrayGrid` vs `BitboardGrid` (Moves per second).
- **AI Solver**: Implement Expectimax or Monte Carlo Tree Search (MCTS) using the optimized bitboard.

## Verification Plan
### Automated Tests
```bash
# We will create a test target
ctest --verbose
```
### Manual Verification
- Verify the build works on your Mac.
- Visual check of the game window and movement animations (if we get to animations).
