#include "machine.h"

#include <chrono>
#include <iostream>

#include "raylib.h"

namespace nes {

Machine::Machine(const std::string &path)
    : bus_(),
      cpu_(bus_),
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

  Image image = GenImageColor(256, 240, WHITE);

  Texture2D texture = LoadTextureFromImage(image);

  while (!WindowShouldClose()) {
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

    if (IsKeyDown(KEY_A)) {
      joypad_.SetKey(Joypad::kLeft, true);
    }
    if (IsKeyReleased(KEY_A)) {
      joypad_.SetKey(Joypad::kLeft, false);
    }

    if (IsKeyDown(KEY_D)) {
      joypad_.SetKey(Joypad::kRight, true);
    }
    if (IsKeyReleased(KEY_D)) {
      joypad_.SetKey(Joypad::kRight, false);
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

    if (IsKeyDown(KEY_J)) {
      joypad_.SetKey(Joypad::kA, true);
    }
    if (IsKeyReleased(KEY_J)) {
      joypad_.SetKey(Joypad::kA, false);
    }

    if (IsKeyDown(KEY_K)) {
      joypad_.SetKey(Joypad::kB, true);
    }
    if (IsKeyReleased(KEY_K)) {
      joypad_.SetKey(Joypad::kB, false);
    }

    bool out = false;
    while (!out) {
      cpu_.Tick();
      int cycles = cpu_.cycles;
      while (--cpu_.cycles > 0);

      for (int i = 0; i < 3 * cycles; ++i) {
        ppu_.Tick();
        if (ppu_.one_frame_finished()) {
          out = true;
        }
      }
    }

    UpdateTexture(texture, ppu_.pixels().data());

    BeginDrawing();
    ClearBackground(GRAY);

    DrawTexturePro(texture,
                   { 0, 0, 256, 240 },
                   { 0, 0, 1.0f * GetRenderWidth(), 1.0f * GetRenderHeight() },
                   { 0, 0 },
                   0.0,
                   WHITE);

    // ppu_.TestRenderNametable(0x2000);
    // ppu_.TestRenderSprite();
    // ppu_.TestPalettes();
    EndDrawing();
  }

  UnloadTexture(texture);
  UnloadImage(image);
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
