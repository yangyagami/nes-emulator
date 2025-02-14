#include <array>
#include <cstdint>
#include <iostream>

#include "cartridge/cartridge.h"

int main() {
  nes::Cartridge cart;
  std::cout << cart.LoadRomFile("nestest.nes") << std::endl;;

  return 0;
}
