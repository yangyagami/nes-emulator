#include "machine.h"

#include <iostream>

#include "raylib.h"

namespace nes {

Machine::Machine(const std::string &path)
    : cpu_(bus_),
      ppu_(cpu_, cartridge_),
      rom_path_(path) {
}

int Machine::Run() {
  const int kSW = 256 * 4;
  const int kSH = 240 * 3;

  InitWindow(kSW, kSH, "nes emulator");
  SetTargetFPS(60);
  SetWindowMinSize(kSW, kSH);
  SetWindowMaxSize(kSW, kSH);

  bus_.Connect(memory_, cartridge_, ppu_, joypad_);

  if (!cartridge_.LoadRomFile(rom_path_)) {
    return -1;
  }

  // See https://www.nesdev.org/wiki/CPU_power_up_state
  cpu_.Reset();
  cpu_.PC = bus_.CpuRead16Bit(0xFFFC);
  cpu_.SP = 0xFD;

  bool debug = false;

  while (!WindowShouldClose()) {
    bool out = false;
    while (!out) {
      if (debug) {
        std::cout << std::format("{:#x} {}\n", cpu_.PC, cpu_.Disassemble(cpu_.PC));
      }
      cpu_.Tick();
      for (int i = 0; i < 3 * cpu_.cycles; ++i) {
        ppu_.Tick();
        if (ppu_.one_frame_finished()) {
          out = true;
        }
      }
      while (--cpu_.cycles > 0);
    }

    if (IsKeyPressed(KEY_P)) {
      debug = !debug;
    }

    if (IsKeyDown(KEY_S)) {
      joypad_.SetKey(Joypad::kDown, true);
    }
    if (IsKeyReleased(KEY_S)) {
      joypad_.SetKey(Joypad::kDown, false);
    }

    if (IsKeyDown(KEY_W)) {
      joypad_.SetKey(Joypad::kUp, true);
    }
    if (IsKeyReleased(KEY_W)) {
      joypad_.SetKey(Joypad::kUp, false);
    }

    if (IsKeyDown(KEY_ENTER)) {
      joypad_.SetKey(Joypad::kStart, true);
    }
    if (IsKeyReleased(KEY_ENTER)) {
      joypad_.SetKey(Joypad::kStart, false);
    }

    if (IsKeyDown(KEY_O)) {
      joypad_.SetKey(Joypad::kSelect, true);
    }
    if (IsKeyReleased(KEY_O)) {
      joypad_.SetKey(Joypad::kSelect, false);
    }

    BeginDrawing();
    ClearBackground(GRAY);

    const float kGameWidth = GetRenderWidth() * 0.80f;

    const float kCellWidth = kGameWidth / 256.0f;
    const float kCellHeight = GetRenderHeight() / 240.0f;

    for (int y = 0; y < 240; ++y) {
      for (int x = 0; x < 256; ++x) {
        Vector2 pos = { x * kCellWidth, y * kCellHeight };
        Vector2 size = { kCellWidth, kCellHeight };
        DrawRectangleV(pos, size, ppu_.pixels()[y * 256 + x]);
      }
    }

    // ppu_.TestRenderNametable(0x2000);
    // ppu_.TestRenderSprite();
    EndDrawing();
  }
  return 0;
}

}  // namespace nes

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: nes-emulator xxx.nes\n";
    return 0;
  }

  nes::Machine m(argv[1]);
  return m.Run();
}
