#include "InputManager.hpp"

namespace Game {

Action InputManager::pollAction() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return Action::Quit;
    } else if (e.type == SDL_KEYDOWN) {
      return translateKey(e.key.keysym.sym);
    }
  }
  return Action::None;
}

Action InputManager::translateKey(SDL_Keycode key) {
  switch (key) {
  case SDLK_UP:
    return Action::Up;
  case SDLK_w:
    return Action::Up; // WASD Support added for free!

  case SDLK_DOWN:
    return Action::Down;
  case SDLK_s:
    return Action::Down;

  case SDLK_LEFT:
    return Action::Left;
  case SDLK_a:
    return Action::Left;

  case SDLK_RIGHT:
    return Action::Right;
  case SDLK_d:
    return Action::Right;

  case SDLK_ESCAPE:
    return Action::Quit;
  case SDLK_r:
    return Action::Restart;
  case SDLK_RETURN:
    return Action::Confirm;
  case SDLK_BACKSPACE:
    return Action::Back;

  default:
    return Action::None;
  }
}

} // namespace Game
