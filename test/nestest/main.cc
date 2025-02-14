#include <iostream>
#include <format>
#include <array>
#include <string>
#include <fstream>

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

  std::ifstream ifs("nestest.log");
  if (!ifs.is_open()) {
    std::cout << "Cannot open nestest.log\n";
    return 255;
  }

  int cycles = 7;

  while (true) {
    std::string line = "";
    std::size_t pos{};
    if (!ifs.eof()) {
      std::getline(ifs, line);
      uint16_t pc{std::stoi(std::string(line, 0, 4), &pos, 16)};

      nes_assert(pc == cpu.PC, std::format("PC assert failed, excepted value: {:#x}, actual: {:#x}", pc, cpu.PC));

      uint8_t a{std::stoi(std::string(line, 50, 2), &pos, 16)};
      nes_assert(a == cpu.A, std::format("A assert failed, excepted value: {:#x}, actual: {:#x}", a, cpu.A));

      uint8_t x{std::stoi(std::string(line, 55, 2), &pos, 16)};
      nes_assert(x == cpu.X, std::format("X assert failed, excepted value: {:#x}, actual: {:#x}", x, cpu.X));

      uint8_t y{std::stoi(std::string(line, 60, 2), &pos, 16)};
      nes_assert(y == cpu.Y, std::format("Y assert failed, excepted value: {:#x}, actual: {:#x}", y, cpu.Y));

      uint8_t p{std::stoi(std::string(line, 65, 2), &pos, 16)};
      nes_assert(p == cpu.P.raw, std::format("P assert failed, excepted value: {:#x}, actual: {:#x}", p, cpu.P.raw));

      int c{std::stoi(std::string(line, 90), &pos)};
      nes_assert(cycles == c, std::format("cycles assert failed, excepted value: {}, actual: {}", c, cycles));
    }
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

    cycles += cpu.cycles;

    while (--cpu.cycles > 0);
  }

  return 0;
}
