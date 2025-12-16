#include "engine/Context.hpp"
#include "engine/Renderer.hpp"
#include "engine/Window.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
  std::cout << "Tile Twister - Starting..." << std::endl;

  try {
    // 1. Initialize SDL (RAII)
    Engine::Context context;

    // 2. Create Window
    Engine::Window window("Tile Twister", 800, 600);

    // 3. Create Renderer
    Engine::Renderer renderer(window);

    // 4. Test Render Loop (Draw distinct color)
    renderer.setDrawColor(0, 128, 0, 255); // Green
    renderer.clear();
    renderer.present();

    std::cout << "Window opened successfully. Waiting 3 seconds..."
              << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "Exiting gracefully." << std::endl;
  return 0;
}
