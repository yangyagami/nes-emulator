#include <iostream>
#include <cstdint>
#include <array>
#include <cassert>

#include "raylib.h"

#include "cpu/cpu.h"
#include "bus/bus.h"

int main() {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  uint8_t tmp[] = { 0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02, 0xa9, 0x08, 0x8d, 0x20, 0x02, 0x8d, 0x21, 0x02 };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  cpu.Tick();
  cpu.Tick();
  cpu.Tick();
  cpu.Tick();
  cpu.Tick();
  cpu.Tick();
  cpu.Tick();
  cpu.Tick();
  cpu.Tick();

  std::printf("A=%02x X=%02x Y=%02x\nPC=%04x\n",
              cpu.A, cpu.X, cpu.Y, cpu.PC);

  std::printf("0200: %02x %02x %02x\n",
              memory[0x0200], memory[0x0201], memory[0x0202]);
  std::printf("0220: %02x %02x\n",
              memory[0x0220], memory[0x0221]);

  // range($0200, $05ff) used to draw pixels.
  int sw = 32 * 20;
  int sh = 32 * 20;
  InitWindow(sw, sh, "easy test");
  SetTargetFPS(60);

  Color colors[] = {
    BLACK,
    BLACK,
    RED,
    GREEN,
    BLUE,
    PURPLE,
    ORANGE,
    YELLOW,
    GRAY,
    LIGHTGRAY,
    WHITE,
  };

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    int x = 0;
    int y = 0;
    for (uint16_t i = 0x0200; i <= 0x05ff; ++i) {
      if (i != 0x0200 && i % 32 == 0) {
        x = 0;
        y++;
      }
      DrawRectangle((x++) * 20, y * 20, 20, 20,
                    colors[memory[i]]);
    }
    EndDrawing();
  }
  return 0;
}
