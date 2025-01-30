#include <iostream>
#include <format>

#include "raylib.h"

#include "cartridge/cartridge.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: tile_test xxx.nes\n";
    return -1;
  }
  nes::Cartridge cart;
  if (!cart.LoadRomFile(argv[1])) {
    std::cout << std::format("No such file: {}\n", argv[1]);
    return -1;
  }

  InitWindow(800, 600, "tile test");
  SetTargetFPS(60);

  const int kCellSize = 2;

  Color colors[] = {
    BLACK,
    WHITE,
    BLUE,
    RED,
  };

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    for (int i = 0; i < cart.chr_rom.size(); i += 16) {
      uint8_t plane0 = cart.chr_rom[i];
      uint8_t plane1 = cart.chr_rom[i + 8];

      for (int j = 7; j >= 0; --j) {
        uint8_t plane0_bit = (plane0 >> j) & 0x1;
        uint8_t plane1_bit = (plane1 >> j) & 0x1;
      }
    }

    EndDrawing();
  }
  return 0;
}
