#include <array>
#include <cstdint>

#include "cartridge/cartridge.h"

int main() {
  std::array<uint8_t, 0x10000> memory = { 0 };
  nes::Cartridge cart("snake.nes", memory);

  return 0;
}
