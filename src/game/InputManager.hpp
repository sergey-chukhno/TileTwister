#pragma once
#include <SDL.h>

namespace Game {

enum class Action { None, Up, Down, Left, Right, Quit, Restart, Confirm, Back };

class InputManager {
public:
  InputManager() = default;
  ~InputManager() = default;

  // Polls for events and translates them into High-Level Actions.
  // Returns Action::None if no relevant event occurred this frame.
  Action pollAction();

private:
  Action translateKey(SDL_Keycode key);
};

} // namespace Game
