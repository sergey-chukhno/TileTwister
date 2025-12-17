cmake_minimum_required(VERSION 3.15)
project(TileTwister VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # Force all dependencies to be static

# --- Dependencies ---
include(FetchContent)

# 1. GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  TLS_VERIFY FALSE
)
# For Windows: Prevent GTest from overriding our compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 2. SDL2
# We try to find it on the system first (e.g. brew install sdl2).
# If not found, we could FetchContent it, but building SDL2 from source can be slow.
# For now, we will rely on find_package. 
# For your Windows teammates, the best approach is actually "vcpkg", 
# but simply fetching it here is a "zero-install" fallback.
# Force Static Build for Simplicity
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
  message(STATUS "SDL2 not found on system, fetching from source...")
  FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-2.28.5.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY FALSE
  )
  FetchContent_MakeAvailable(SDL2)
  # SDL2 CMake creates 'SDL2::SDL2-static' or 'SDL2::SDL2' depending on settings
  if(TARGET SDL2-static)
    add_library(SDL2::SDL2 ALIAS SDL2-static)
  elseif(TARGET SDL2)
    add_library(SDL2::SDL2 ALIAS SDL2)
  endif()
  # If SDL2 provides the SDL2main target (required on some Windows setups), alias it too
  if(TARGET SDL2main)
    add_library(SDL2::SDL2main ALIAS SDL2main)
  endif()
endif()

# 3. Freetype (Required for SDL2_ttf)
# Try to automatically locate MSYS2 / MinGW installations on Windows and
# predefine variables that FindFreetype will use. This helps CMake find
# freetype when MSYS2 provided libraries are available (e.g. C:/msys64/ucrt64).
if(WIN32)
  set(_msys_candidates
    "C:/msys64/ucrt64"
    "C:/msys64/mingw64"
    "C:/msys64/mingw32"
  )
  foreach(_p IN LISTS _msys_candidates)
    if(EXISTS "${_p}")
      # Look for ft2build.h inside common include locations
      find_path(_ft_inc NAMES ft2build.h PATHS
        ${_p}/include
        ${_p}/include/freetype2
        ${_p}/include/freetype2/freetype
        NO_DEFAULT_PATH
      )
      if(_ft_inc)
        # Configure variables that FindFreetype and other modules expect
        set(FREETYPE_INCLUDE_DIRS "${_ft_inc}" CACHE PATH "Freetype include dir (auto)" FORCE)
        # Try common library names/locations
        find_library(_ft_lib NAMES freetype libfreetype-6 freetype6 PATHS
          ${_p}/lib
          ${_p}/lib64
          NO_DEFAULT_PATH
        )
        if(_ft_lib)
          set(FREETYPE_LIBRARY "${_ft_lib}" CACHE FILEPATH "Freetype library (auto)" FORCE)
        endif()
        # Add prefix to CMAKE_PREFIX_PATH to help find other deps (harfbuzz, etc.)
        list(APPEND CMAKE_PREFIX_PATH "${_p}")
        break()
      endif()
    endif()
  endforeach()
endif()

find_package(Freetype QUIET)
if(NOT Freetype_FOUND)
  message(STATUS "Freetype not found, fetching from source...")
  FetchContent_Declare(
      Freetype
      GIT_REPOSITORY https://github.com/freetype/freetype.git
      GIT_TAG VER-2-13-2
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY FALSE
  )
  FetchContent_MakeAvailable(Freetype)
  if(TARGET freetype)
      add_library(Freetype::Freetype ALIAS freetype)
  endif()
endif()

# 4. SDL2_ttf
# Disable vendored freetype to use our modern fetched one
set(SDL2TTF_VENDORED OFF CACHE BOOL "" FORCE) 

find_package(SDL2_ttf QUIET)
if(NOT SDL2_ttf_FOUND)
  message(STATUS "SDL2_ttf not found on system, fetching from source...")
  FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.22.0
  )
  FetchContent_MakeAvailable(SDL2_ttf)
  if(TARGET SDL2_ttf::SDL2_ttf)
     # Good
  elseif(TARGET SDL2_ttf)
    add_library(SDL2_ttf::SDL2_ttf ALIAS SDL2_ttf)
  endif()
endif()

# --- Project Sources ---
# We will define libraries for Core and Engine to enforce our architecture

# Core Library (Pure C++)
add_library(TileTwister_Core
    src/core/Tile.cpp
    src/core/Grid.cpp
    src/core/GameLogic.cpp
    src/core/dummy.cpp # Placeholder until we have real files
)
target_include_directories(TileTwister_Core PUBLIC src/core)

# Engine Library (SDL dependents)
add_library(TileTwister_Engine
    src/engine/Window.cpp
    src/engine/Renderer.cpp
    src/engine/Font.cpp
    src/engine/dummy.cpp # Placeholder
)
target_include_directories(TileTwister_Engine PUBLIC src/engine)
target_link_libraries(TileTwister_Engine PUBLIC SDL2::SDL2 SDL2_ttf::SDL2_ttf TileTwister_Core)

# Game Executable
add_executable(TileTwister
    src/main.cpp
    src/game/Game.cpp
    src/game/InputManager.cpp
    src/game/dummy.cpp
)
target_include_directories(TileTwister PUBLIC src)
target_link_libraries(TileTwister PRIVATE TileTwister_Core TileTwister_Engine)

# On Windows/MinGW we must link SDL2's SDL2main implementation into the executable
# to provide the WinMain entry point expected by the Windows subsystem.
if(TARGET SDL2::SDL2main)
  target_link_libraries(TileTwister PRIVATE SDL2::SDL2main)
endif()

# --- Testing ---
enable_testing()
add_executable(TileTwister_Tests 
    tests/main_test.cpp
    tests/core/Tile_test.cpp
    tests/core/Grid_test.cpp
    tests/core/GameLogic_test.cpp
)
target_link_libraries(TileTwister_Tests PRIVATE GTest::gtest_main TileTwister_Core)

include(GoogleTest)
gtest_discover_tests(TileTwister_Tests)
