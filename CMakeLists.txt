cmake_minimum_required(VERSION 3.15)
project(TileTwister VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Dependencies ---
include(FetchContent)

# 1. GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
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
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
  message(STATUS "SDL2 not found on system, fetching from source...")
  FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-2.28.5.zip
  )
  FetchContent_MakeAvailable(SDL2)
  # SDL2 CMake creates 'SDL2::SDL2-static' or 'SDL2::SDL2' depending on settings
  if(TARGET SDL2-static)
    add_library(SDL2::SDL2 ALIAS SDL2-static)
  elseif(TARGET SDL2)
    add_library(SDL2::SDL2 ALIAS SDL2)
  endif()
endif()

# --- Project Sources ---
# We will define libraries for Core and Engine to enforce our architecture

# Core Library (Pure C++)
add_library(TileTwister_Core
    src/core/dummy.cpp # Placeholder until we have real files
)
target_include_directories(TileTwister_Core PUBLIC src/core)

# Engine Library (SDL dependents)
add_library(TileTwister_Engine
    src/engine/dummy.cpp # Placeholder
)
target_include_directories(TileTwister_Engine PUBLIC src/engine)
target_link_libraries(TileTwister_Engine PRIVATE SDL2::SDL2)

# Game Executable
add_executable(TileTwister
    src/main.cpp
    src/game/dummy.cpp
)
target_include_directories(TileTwister PUBLIC src)
target_link_libraries(TileTwister PRIVATE TileTwister_Core TileTwister_Engine)

# --- Testing ---
enable_testing()
add_executable(TileTwister_Tests tests/main_test.cpp)
target_link_libraries(TileTwister_Tests PRIVATE GTest::gtest_main TileTwister_Core)

include(GoogleTest)
gtest_discover_tests(TileTwister_Tests)
