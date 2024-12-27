#ifndef NES_EMULATOR_BUS_BUS_H_
#define NES_EMULATOR_BUS_BUS_H_

#include <array>
#include <cstdint>

namespace nes {
class Bus {
 public:
  Bus(std::array<uint8_t, 0xFFFF> &memory);
  void CpuWrite8Bit(uint16_t address, uint8_t value);

  void CpuWrite16Bit(uint16_t address, uint16_t value);

  uint8_t CpuRead8Bit(uint16_t address);

  uint16_t CpuRead16Bit(uint16_t address);

 private:
  std::array<uint8_t, 0xFFFF> &memory_;
};
}  // namespace nes

#endif  // NES_EMULATOR_BUS_BUS_H_
