#ifndef NES_EMULATOR_BUS_BUS_H_
#define NES_EMULATOR_BUS_BUS_H_

#include <array>
#include <cstdint>

#include "cartridge/cartridge.h"
#include "ppu/ppu.h"
#include "joypad/joypad.h"

namespace nes {
class Bus {
 public:
  void Connect(std::array<uint8_t, 0x0800> &memory, Cartridge &cartridge, PPU &ppu, Joypad &joypad);

  void CpuWrite8Bit(uint16_t address, uint8_t value);

  void CpuWrite16Bit(uint16_t address, uint16_t value);

  uint8_t CpuRead8Bit(uint16_t address);

  uint16_t CpuRead16Bit(uint16_t address);

 private:
  std::array<uint8_t, 0x0800> *memory_;
  Cartridge *cartridge_;
  PPU *ppu_;
  Joypad *joypad_;
};
}  // namespace nes

#endif  // NES_EMULATOR_BUS_BUS_H_
