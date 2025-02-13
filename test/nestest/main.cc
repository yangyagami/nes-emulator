#include <iostream>
#include <format>
#include <array>

#include "bus/bus.h"
#include "cpu/cpu.h"
#include "ppu/ppu.h"
#include "cartridge/cartridge.h"
#include "joypad/joypad.h"

using namespace nes;

int main() {
  std::array<uint8_t, 0x0800> memory;
  Cartridge cartridge;
  Joypad joypad;
  Bus bus;
  Cpu cpu(bus);
  PPU ppu(cpu, cartridge);
  bus.Connect(memory, cartridge, ppu, joypad);

  cartridge.LoadRomFile("nestest.nes");

  cpu.Reset();
  cpu.PC = bus.CpuRead16Bit(0xFFFC);
  cpu.SP = 0xFD;

  while (true) {
    std::cout << std::format("{:#x} {}\n", cpu.PC, cpu.Disassemble(cpu.PC));
    cpu.Tick();
    for (int i = 0; i < 3 * cpu.cycles; ++i) {
      ppu.Tick();
    }
    while (--cpu.cycles > 0);
  }

  return 0;
}
