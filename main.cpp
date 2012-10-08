#include "renderer.h"

int main(int argc, char *argv[]) {
  Renderer renderer = Renderer();
  renderer.init(argc, argv);
  setActiveRenderer(&renderer);
  renderer.mainLoop();
}
