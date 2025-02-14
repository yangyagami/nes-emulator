#include <iostream>
#include <format>
#include <array>

#include "bus/bus.h"
#include "cpu/cpu.h"
#include "ppu/ppu.h"
#include "cartridge/cartridge.h"
#include "joypad/joypad.h"
#include "utils/assert.h"

using namespace nes;

int main() {
  std::array<uint8_t, 0x0800> memory = { 0 };
  Cartridge cartridge;
  Joypad joypad;
  Bus bus;
  Cpu cpu(bus);
  PPU ppu(cpu, cartridge);
  bus.Connect(memory, cartridge, ppu, joypad);

  cartridge.LoadRomFile("nestest.nes");

  cpu.Reset();
  cpu.PC = 0xC000;
  cpu.SP = 0xFD;

  while (true) {
    std::cout << std::format("${:04x} {:10} A:${:02x} X:${:02x} Y:${:02x} P:${:02x} SP:${:02x}\n",
                             cpu.PC,
                             cpu.Disassemble(cpu.PC),
                             cpu.A,
                             cpu.X,
                             cpu.Y,
                             cpu.P.raw,
                             cpu.SP);
    nes_assert(memory[0x2] == 0, "");
    cpu.Tick();
    while (--cpu.cycles > 0);
  }

  return 0;
}
