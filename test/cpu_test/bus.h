#ifndef NES_EMULATOR_BUS_BUS_H_
#define NES_EMULATOR_BUS_BUS_H_

#include <array>
#include <cstdint>

namespace nes {
class Bus {
 public:
  Bus(std::array<uint8_t, 0x10000> &memory);

  void CpuWrite8Bit(uint16_t address, uint8_t value);

  void CpuWrite16Bit(uint16_t address, uint16_t value);

  uint8_t CpuRead8Bit(uint16_t address);

  uint16_t CpuRead16Bit(uint16_t address);

 private:
  std::array<uint8_t, 0x10000> &memory_;
};

Bus::Bus(std::array<uint8_t, 0x10000> &memory) : memory_(memory) {}

void Bus::CpuWrite8Bit(uint16_t address, uint8_t value) {
  // assert(address < 0x0800);
  memory_[address] = value;
}

void Bus::CpuWrite16Bit(uint16_t address, uint16_t value) {
  // assert(address < 0x0800 && address + 1 < 0x0800);
  memory_[address] = value;
  memory_[address + 1] = value >> 8;
}

uint8_t Bus::CpuRead8Bit(uint16_t address) {
  // assert(address < 0x0800);
  return memory_[address];
}

uint16_t Bus::CpuRead16Bit(uint16_t address) {
  // assert(address < 0x0800 && address + 1 < 0x0800);
  uint16_t ret = memory_[address];
  ret |= (memory_[address + 1] << 8);
  return ret;
}

}  // namespace nes

#endif  // NES_EMULATOR_BUS_BUS_H_
