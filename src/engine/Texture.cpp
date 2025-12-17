#include "Texture.hpp"
#include "Renderer.hpp"
#include <SDL_image.h>
#include <stdexcept>

namespace Engine {

Texture::Texture(Renderer &renderer, const std::string &path)
    : m_texture(nullptr), m_width(0), m_height(0) {

  SDL_Surface *surface = IMG_Load(path.c_str());
  if (!surface) {
    throw std::runtime_error("IMG_Load failed: " + std::string(IMG_GetError()));
  }

  m_texture = SDL_CreateTextureFromSurface(renderer.getInternal(), surface);
  if (!m_texture) {
    SDL_FreeSurface(surface);
    throw std::runtime_error("CreateTexture failed: " +
                             std::string(SDL_GetError()));
  }

  m_width = surface->w;
  m_height = surface->h;
  SDL_FreeSurface(surface);
}

Texture::~Texture() {
  if (m_texture) {
    SDL_DestroyTexture(m_texture);
  }
}

Texture::Texture(Texture &&other) noexcept
    : m_texture(other.m_texture), m_width(other.m_width),
      m_height(other.m_height) {
  other.m_texture = nullptr;
}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    if (m_texture)
      SDL_DestroyTexture(m_texture);
    m_texture = other.m_texture;
    m_width = other.m_width;
    m_height = other.m_height;
    other.m_texture = nullptr;
  }
  return *this;
}

void Texture::setColor(uint8_t r, uint8_t g, uint8_t b) {
  if (m_texture)
    SDL_SetTextureColorMod(m_texture, r, g, b);
}

void Texture::setAlpha(uint8_t a) {
  if (m_texture)
    SDL_SetTextureAlphaMod(m_texture, a);
}

} // namespace Engine
