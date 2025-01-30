#include "machine.h"

#include <iostream>

#include "raylib.h"

namespace nes {

Machine::Machine(const std::string &path)
    : bus_(memory_, cartridge_, ppu_),
      cpu_(bus_),
      ppu_(cpu_),
      rom_path_(path) {
}

int Machine::Run() {
  InitWindow(800, 600, "nes emulator");
  SetTargetFPS(60);

  if (!cartridge_.LoadRomFile(rom_path_)) {
    return -1;
  }

  // See https://www.nesdev.org/wiki/CPU_power_up_state
  cpu_.Reset();
  cpu_.PC = bus_.CpuRead16Bit(0xFFFC);
  cpu_.SP = 0xFD;

  while (!WindowShouldClose()) {
    bool out = false;
    while (!out) {
      cpu_.Tick();
      for (int i = 0; i < 3 * cpu_.cycles; ++i) {
        ppu_.Tick();
        if (ppu_.one_frame_finished()) {
          out = true;
        }
      }
      while(--cpu_.cycles > 0);
    }
    BeginDrawing();
    ClearBackground(BLACK);
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
