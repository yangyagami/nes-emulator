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

    int x = 0;
    int y = 0;
    int tiles = 0;
    // for (int i = 0; i < cart.chr_rom.size(); i += 16) {
    for (int i = 0; i < 512; ++i) {
      // tile_no * 16
      for (int j = i * 16; j < i * 16 + 8; ++j) {
        uint8_t plane0 = cart.chr_rom[j];
        uint8_t plane1 = cart.chr_rom[j + 8];

        for (int k = 7; k >= 0; --k) {
          uint8_t plane0_bit = (plane0 >> k) & 0x1;
          uint8_t plane1_bit = (plane1 >> k) & 0x1;
          uint8_t color_idx = plane0_bit + plane1_bit * 2;

          DrawRectangle(x + (7 - k) * kCellSize, y + (j - i * 16) * kCellSize,
                        kCellSize, kCellSize, colors[color_idx]);
        }
      }
      tiles++;
      x += 8 * kCellSize;
      if (tiles % 16 == 0) {
        x = 0;
        y += 8 * kCellSize;
      }
    }

    EndDrawing();
  }
  return 0;
}
