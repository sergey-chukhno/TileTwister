#include "engine/Context.hpp"
#include "engine/Renderer.hpp"
#include "engine/Window.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "Tile Twister - Starting..." << std::endl;

  try {
    // 1. Initialize SDL (RAII)
    Engine::Context context;

    // 2. Create Window
    Engine::Window window("Tile Twister", 800, 600);

    // 3. Create Renderer
    Engine::Renderer renderer(window);

    std::cout << "Window opened successfully. Running Game Loop..."
              << std::endl;

    // 4. SDL Event Loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
      // Handle Events
      while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
          quit = true;
        }
      }

      // Render
      renderer.setDrawColor(0, 128, 0, 255); // Green
      renderer.clear();
      renderer.present();

      // Optimization: Small delay to prevent 100% CPU usage on empty loop
      // In a real game, this would be vertical sync or frame capping.
      SDL_Delay(16); // ~60 FPS
    }

  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "Exiting gracefully." << std::endl;
  return 0;
}
