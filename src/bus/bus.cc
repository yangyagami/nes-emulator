#include "bus.h"

#include <cstdint>
#include <array>
#include <cassert>

namespace nes {
Bus::Bus(std::array<uint8_t, 0x0800> &memory) : memory_(memory) {}

void Bus::CpuWrite8Bit(uint16_t address, uint8_t value) {
  memory_[address] = value;
}

void Bus::CpuWrite16Bit(uint16_t address, uint16_t value) {
  memory_[address] = value;
  memory_[address + 1] = value >> 8;
}

uint8_t Bus::CpuRead8Bit(uint16_t address) {
  return memory_[address];
}

uint16_t Bus::CpuRead16Bit(uint16_t address) {
  uint16_t ret = memory_[address];
  ret |= (memory_[address + 1] << 8);
  return ret;
}

}  // namespace nes
