#include <array>
#include <cstdint>
#include <iostream>

#include "cartridge/cartridge.h"

int main() {
  nes::Cartridge cart;
  std::cout << cart.LoadRomFile("snake.nes") << std::endl;;

  return 0;
}
