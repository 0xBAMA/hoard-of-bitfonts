#include "spaceship.h"

int main(int argc, char const *argv[]) {
  ssGenerator shipyard;

  shipyard.populate();
  shipyard.genRandomEngine();
  shipyard.genPalette();

  shipyard.num_ops = 7;

  shipyard.minxyScale = 1;
  shipyard.maxxyScale = 2;

  shipyard.minzScale = 7;
  shipyard.maxzScale = 15;

  shipyard.genSpaceship();
  shipyard.dump();

  return 0;
}
