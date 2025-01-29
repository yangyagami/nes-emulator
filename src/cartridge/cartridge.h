#ifndef NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_
#define NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_

#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <format>

namespace nes {

struct Cartridge {
  // See https://www.nesdev.org/wiki/INES#Flags_6
  union {
    struct {
      uint8_t NAMETABLE_ARRANGEMENT : 1;
      uint8_t PRG_RAM : 1;
      uint8_t TRAINER : 1;
      uint8_t ALTERNATIVE_NAMETABLE : 1;
      uint8_t MAPPER_NUMBER : 4;
    };
    uint8_t raw;
  } Flags6;

  std::vector<uint8_t> prg_rom;
  std::vector<uint8_t> chr_rom;

 public:
  bool LoadRomFile(const std::string &path);
};

}  // namespace nes

#endif  // NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_
