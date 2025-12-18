#pragma once
#include <SDL.h>
#include <string>

namespace Engine {

class Renderer; // Forward declaration

class Texture {
public:
  // Constructor loads texture from file
  Texture(Renderer &renderer, const std::string &path);
  ~Texture();

  // No copy
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  // Move allowed
  Texture(Texture &&other) noexcept;
  Texture &operator=(Texture &&other) noexcept;

  [[nodiscard]] SDL_Texture *get() const { return m_texture; }
  [[nodiscard]] int getWidth() const { return m_width; }
  [[nodiscard]] int getHeight() const { return m_height; }

  // Set color modulation (tint)
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void setAlpha(uint8_t a);

private:
  SDL_Texture *m_texture;
  int m_width;
  int m_height;
};

} // namespace Engine
