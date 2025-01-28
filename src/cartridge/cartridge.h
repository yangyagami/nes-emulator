#ifndef NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_
#define NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_

#include <array>
#include <string>

namespace nes {

class Cartridge {
 public:
  Cartridge(const std::string &path, std::array<uint8_t, 0x10000> &memory);
};

}  // namespace nes

#endif  // NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_
