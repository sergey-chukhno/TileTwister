# Tile Twister - Project Rules & Guidelines

## 1. Project Overview
**Goal**: Create a robust, modular, and professional-grade implementation of the game "2048" in C++.
**Core Technologies**: C++17/20, SDL2 (for rendering).
**Core Technologies**: C++17/20, SDL2, CMake, GoogleTest.
**Phases**:
1.  **Core Logic**: Platform-agnostic game engine (Grid, Tiles, Merging logic).
2.  **Visualization**: SDL-based rendering engine.

## 2. Coding Standards (Modern C++)
All code must adhere to **Systematic C++** principles.

### General
-   **Standard Version**: C++17 minimum, C++20 preferred where supported.
-   **Language**: English for all code, comments, and commit messages.

### Resource Management (RAII)
-   **No raw `new`/`delete`**. Use `std::unique_ptr` for exclusive ownership and `std::shared_ptr` only when ownership is truly shared.
-   **Rule of Zero/Five**: Classes dealing with resources must explicitly define or delete copy/move constructors and assignment operators.
-   **Deterministic Destruction**: relying on destructors for cleanup (textures, windows, surfaces).

### Safety & Correctness
-   **Const Correctness**: Mark methods `const` if they don't modify logical state. Use `const` variables for immutables.
-   **Strong Typing**: Avoid «primitive obsession». Use strong types or `enum class` for states, directions, etc.
-   **Null Safety**: Avoid `nullptr` checks scattered everywhere. Use references `&` when an object must exist.
-   **No Magic Numbers**: Define constants for grid size, colors, window dimensions.

### Project Structure (Separation of Concerns)
-   **Core Logic**: `src/core` - Pure C++, no SDL dependencies. Testable in isolation.
-   **Engine/View**: `src/engine` - SDL wrappers, Window management, Rendering loop.
-   **Game/App**: `src/game` - Glues Core and Engine together.

## 3. Architecture Principles
-   **Model-View Separation**: The game logic (Grid, Tile) must not know about the rendering (SDL_Texture, Window).
-   **Event-Driven**: Input handling should be decoupled from game logic updates.
-   **Testability**: The core logic must be verifiable via unit tests without opening a window.

## 4. Mentor Persona Rules
-   **Explain "Why"**: Never just give code. Explain the design pattern or C++ feature being used.
-   **Incremental Implementation**: Small, verifiable steps.
-   **Socratic Method**: Ask questions to prompt the user's understanding before providing answers.
-   **Professionalism**: Treat this as a production codebase, not a toy project.

## 5. Portability & Build System (User Decisions)
-   **Build System**: CMake (Minimum 3.15).
-   **Testing Framework**: GoogleTest (via `FetchContent` or `vcpkg`).
-   **Dependencies**: Manage via **vcpkg** (recommended) or `FetchContent`.
    -   *Why not Docker?* Docker is excellent for servers/CLI, but poor for GUI applications (SDL) due to OS-specific windowing and GPU driver passthrough complexities.
    -   *Strategy*: CMake generates native implementation (VS Solution on Windows, Makefiles on Mac). We write standard C++ and use SDL2 to abstract the OS window logic.

## 6. Specific Constraints (from TileTwister.md)
-   **Grid**: 4x4.
-   **Spawning**: Random 2 or 4.
-   **Movement**: Standard 2048 sliding/merging rules.
-   **Validation**: Must be able to simulate scenarios (unit/integration tests) to verify logic.
