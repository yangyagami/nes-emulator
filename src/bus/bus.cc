#include "bus.h"

#include <cstdint>
#include <array>
#include <cassert>

#include "cartridge/cartridge.h"
#include "utils/assert.h"

namespace nes {
Bus::Bus(std::array<uint8_t, 0x0800> &memory, Cartridge &cartridge, PPU &ppu)
    : memory_(memory), cartridge_(cartridge), ppu_(ppu) {
}

void Bus::CpuWrite8Bit(uint16_t address, uint8_t value) {
  if (address <= 0x0FFF) {
    memory_[address - 0x0800] = value;
  } else if (address >= 0x1000 && address <= 0x17FF) {
    memory_[address - 0x1000] = value;
  } else if (address >= 0x1800 && address <= 0x1FFF) {
    memory_[address - 0x1800] = value;
  } else if (address >= 0x2000 && address <= 0x2007) {
    // PPU
    ppu_.Write(address, value);
  } else if (address >= 0x4020) {
    // Cartridge
    nes_assert(false, std::format("Unsupported write: {:#x}", address));
  } else {
    if (address >= memory_.size()) {
      nes_assert(false, std::format("Out of memory: {:#x}", address));
    }
    memory_[address] = value;
  }
}

void Bus::CpuWrite16Bit(uint16_t address, uint16_t value) {
  CpuWrite8Bit(address, value);
  CpuWrite8Bit(address + 1, value >> 8);
}

uint8_t Bus::CpuRead8Bit(uint16_t address) {
  if (address <= 0x0FFF) {
    return memory_[address - 0x0800];
  } else if (address >= 0x1000 && address <= 0x17FF) {
    return memory_[address - 0x1000];
  } else if (address >= 0x1800 && address <= 0x1FFF) {
    return memory_[address - 0x1800];
  } else if (address >= 0x4020) {
    // Cartridge
    if (address >= 0x8000) {
      if (cartridge_.mapper == 0) {
        // See https://www.nesdev.org/wiki/NROM
        if (cartridge_.prg_rom.size() == 16384) {
          if (address <= 0xBFFF) {
            return cartridge_.prg_rom[address - 0x8000];
          } else {
            return cartridge_.prg_rom[address - 0xC000];
          }
        } else {
          nes_assert(false, std::format("Unsupported NROM-256"));
        }
      } else {
        nes_assert(false, std::format("Unsupported mapper: {:#x}",
                                      cartridge_.mapper));
      }
    } else {
      nes_assert(false, std::format("Unsupported access: {:#x}", address));
    }
  } else {
    if (address >= memory_.size()) {
      nes_assert(false, std::format("Out of memory: {:#x}", address));
    }
    return memory_[address];
  }
}

uint16_t Bus::CpuRead16Bit(uint16_t address) {
  uint16_t ret = CpuRead8Bit(address);
  ret |= (CpuRead8Bit(address + 1) << 8);
  return ret;
}

}  // namespace nes
