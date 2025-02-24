#include <array>
#include <cstdint>
#include <iostream>

#include "cartridge/cartridge.h"

int main() {
  nes::Cartridge cart;
  std::cout << cart.LoadRomFile("ice.nes") << std::endl;;

  return 0;
}
