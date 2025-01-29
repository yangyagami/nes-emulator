#include "machine.h"

#include "raylib.h"

namespace nes {

Machine::Machine(const std::string &path)
    : bus_(memory_, cartridge_),
      cpu_(bus_),
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
    while (true) {
      for (int i = 0; i < 3; ++i) {
        ppu_.Tick();
      }
      cpu_.Tick();
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
