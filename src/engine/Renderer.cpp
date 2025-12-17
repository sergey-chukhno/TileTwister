#include "Renderer.hpp"
#include <stdexcept>
#include <string>

namespace Engine {

Renderer::Renderer(const Window &window) : renderer(nullptr) {
  renderer =
      SDL_CreateRenderer(window.getNativeHandle(), -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    throw std::runtime_error("Renderer could not be created! SDL_Error: " +
                             std::string(SDL_GetError()));
  }
}

Renderer::~Renderer() {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
}

Renderer::Renderer(Renderer &&other) noexcept : renderer(other.renderer) {
  other.renderer = nullptr;
}

Renderer &Renderer::operator=(Renderer &&other) noexcept {
  if (this != &other) {
    if (renderer)
      SDL_DestroyRenderer(renderer);
    renderer = other.renderer;
    other.renderer = nullptr;
  }
  return *this;
}

void Renderer::clear() { SDL_RenderClear(renderer); }

void Renderer::present() { SDL_RenderPresent(renderer); }

void Renderer::setDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Renderer::drawFillRect(int x, int y, int w, int h) {
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_RenderFillRect(renderer, &rect);
}

} // namespace Engine
