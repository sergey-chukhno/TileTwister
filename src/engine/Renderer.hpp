#pragma once
#include "Tile.hpp" // Will need this later to render tiles
#include "Window.hpp"
#include <SDL.h>

namespace Engine {

class Renderer {
public:
  explicit Renderer(const Window &window);
  ~Renderer();

  // No copy
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  // Move allowed
  Renderer(Renderer &&other) noexcept;
  Renderer &operator=(Renderer &&other) noexcept;

  // Drawing Primitives
  void clear();
  void present();
  void setDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

  // Future methods:
  // void drawRect(int x, int y, int w, int h);
  // void drawTile(const Core::Tile& tile, int x, int y);

private:
  SDL_Renderer *renderer;
};

} // namespace Engine
