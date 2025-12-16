#pragma once
#include <SDL.h>
#include <stdexcept>
#include <string>

namespace Engine {

/**
 * @brief RAII wrapper for SDL Global State (Init/Quit)
 */
class Context {
public:
  Context() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error("SDL could not initialize! SDL_Error: " +
                               std::string(SDL_GetError()));
    }
  }

  ~Context() { SDL_Quit(); }

  // Delete copy/move to ensure only one instance of global SDL state logic
  // exists (singleton-like behavior)
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator=(Context &&) = delete;
};

} // namespace Engine
