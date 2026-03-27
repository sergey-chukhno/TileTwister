## 1. Prerequisites (WSL Setup)

Ensure your WSL is up to date. Open PowerShell or Command Prompt on Windows and run:
```powershell
wsl --update
```

### SANITY CHECK: Are you really in WSL?
Open your terminal and run:
```bash
uname -a
```
If you see **"microsoft"** or **"Linux"** in the output, you are in WSL.
If you see **"Windows"** or paths like **"C:/Program Files"**, you are in the **wrong terminal**. Switch to "Ubuntu" or your WSL distro.

*IMPORTANT: Do NOT mix Windows and WSL build environments. Run all build commands strictly within the WSL terminal.*

## 2. Install Build Dependencies

Open your WSL terminal and install the necessary compiler and build tools:

```bash
sudo apt update
sudo apt install -y build-essential cmake git
```

### Install SDL2 Libraries (CRITICAL STEP)
The project **requires** these system libraries. Installing them via `apt` prevents slow source builds and avoids "CMake Error: Compatibility with CMake < 3.5" errors seen when building Freetype from source.

```bash
sudo apt update
sudo apt install -y \
    libsdl2-dev \
    libsdl2-ttf-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libfreetype6-dev
```

## 3. Configure and Build

Navigate to your workspace **inside the WSL filesystem** (e.g., `~/TileTwister`) for best performance and compatibility.

```bash
# 1. Create and enter build directory
mkdir build && cd build

# 2. Configure the project
cmake ..

# 3. Build (Compile) the project
cmake --build . -j$(nproc)
```

## 4. Launching the Game

The executable is named `TileTwister`. Run it from the **project root**:

```bash
cd ..
./build/TileTwister
```

## 5. Troubleshooting & Common Issues

### "CMake Error: The current CMakeCache.txt directory is different..."
This happens if you run `cmake` from a Windows tool (like VS Code) and then try `make` in WSL, or if you are building on a mounted Windows drive (`/mnt/c/...`).

**The Fix (Clean Build):**
Delete your build folder and start fresh **strictly** from the WSL terminal:
```bash
cd ..
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

### No GUI / Window fails to open
If you get `Could not initialize SDL: No available video device`:
- Ensure WSLg is active. Try running `xclock`.
- If on an older system, you may need: `export DISPLAY=:0`

### Audio Issues
If the game launches but there is no sound, ensure `pulseaudio` or `alsa` is configured. Most modern WSLg setups handle audio automatically.

### Running Tests
To verify the build, you can run the unit and integration tests:
```bash
./build/TileTwister_Tests
./build/IntegrationTests
```
