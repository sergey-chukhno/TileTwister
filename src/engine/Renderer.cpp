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

void Renderer::clear() {
  // Default background color (e.g., light beige for 2048)
  // For now, let's just clear. The user should set color before clearing if
  // needed, or we define a default "clear color".
  SDL_RenderClear(renderer);
}

void Renderer::present() { SDL_RenderPresent(renderer); }

void Renderer::setDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

} // namespace Engine
