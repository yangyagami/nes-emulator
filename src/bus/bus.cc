#include "bus.h"

#include <cstdint>
#include <array>
#include <cassert>

#include "cartridge/cartridge.h"

namespace nes {
Bus::Bus(std::array<uint8_t, 0x0800> &memory, Cartridge &cartridge)
    : memory_(memory), cartridge_(cartridge) {
}

void Bus::CpuWrite8Bit(uint16_t address, uint8_t value) {
  if (address <= 0x0FFF) {
    memory_[address - 0x0800] = value;
  } else if (address >= 0x1000 && address <= 0x17FF) {
    memory_[address - 0x1000] = value;
  } else if (address >= 0x1800 && address <= 0x1FFF) {
    memory_[address - 0x1800] = value;
  } else {
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
  } else {
    return memory_[address];
  }
}

uint16_t Bus::CpuRead16Bit(uint16_t address) {
  uint16_t ret = CpuRead8Bit(address);
  ret |= (CpuRead8Bit(address + 1) << 8);
  return ret;
}

}  // namespace nes
