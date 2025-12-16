# Tile Twister

A modern C++ implementation of the 2048 game using SDL2.

## Project Structure
*   `src/core`: Pure C++ game logic (Platform independent).
*   `src/engine`: SDL2 wrappers (Graphics/Windowing).
*   `src/game`: Main application loop.
*   `tests/`: GoogleTest suite.

## How to Build

We use **CMake**, which ensures you can build this project on Windows, macOS, and Linux without changing any code.

### 🍎 For macOS Users
**Prerequisites**: `cmake`, `ninja` (optional but recommended), `xcode-select` installed.

```bash
# 1. Generate build files (using Ninja is faster)
cmake -S . -B build -G Ninja

# 2. Build the project
cmake --build build

# 3. Run the game
./build/TileTwister

# 4. Run tests
cd build && ctest --verbose
```

### 🪟 For Windows Users (Team Guide)

You do **NOT** need to manually download SDL2 or GoogleTest. CMake handles this for you.

**Prerequisites**:
1.  **Visual Studio 2022** (Community Edition is free).
    *   *During installation, ensure "Desktop development with C++" workload is selected.*
2.  **CMake** (Usually included in Visual Studio, or install from cmake.org).
3.  **Git**.

#### Option A: Using Visual Studio (Easiest)
1.  Open Visual Studio.
2.  Select **"Open a local folder"**.
3.  Navigate to the `TileTwister` folder.
4.  Visual Studio will detect `CMakeLists.txt` and automatically configure the project.
    *   *Watch the "Output" window to see it downloading SDL2/GTest.*
5.  Select `TileTwister.exe` from the startup item dropdown (green play button).
6.  Press **F5** to Build & Run.

#### Option B: Command Line (PowerShell)
```powershell
# 1. Create build directory
cmake -S . -B build

# 2. Build (generates a Debug build by default)
cmake --build build --config Debug

# 3. Run
.build/TileTwister.exe

# 4. Run Tests
cd build
ctest -C Debug
```

## Troubleshooting Windows Builds
*   **"SDL2 not found"**: Ensure you have an internet connection so CMake can fetch the dependencies from GitHub.
*   **Console Window**: On Windows, the game might spawn a console window alongside the game window. This is normal for debug builds.
