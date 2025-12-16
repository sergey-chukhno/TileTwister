# Implementation Plan - Tile Twister

# Goal
Build a C++2048 clone using modern C++17/20 and SDL2, focusing on clean architecture and testability.

## User Review Required
> [!NOTE]
> **Portability strategy**: We will use CMake + a package manager (likely `vcpkg` or CMake's `FetchContent`) to handle SDL2 and GTest. This ensures your Windows teammates can just run `cmake` and it works, without needing Docker containers which struggle with GUI apps.

## Proposed Milestones

### Milestone 1: Project Skeleton & Build System
**Goal**: A compiling "Hello World" that links GTest and SDL2.
- Setup `CMakeLists.txt`
- Configure `GoogleTest`
- Configure `SDL2`
- Verify compiling on Mac (User) and ensure CMake scripts are standard for Windows.

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

## Verification Plan
### Automated Tests
```bash
# We will create a test target
ctest --verbose
```
### Manual Verification
- Verify the build works on your Mac.
- Visual check of the game window and movement animations (if we get to animations).
