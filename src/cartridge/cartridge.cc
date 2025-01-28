#include "cartridge.h"

#include <array>
#include <string>
#include <fstream>
#include <cstdint>

namespace nes {

Cartridge::Cartridge(const std::string &path, std::array<uint8_t, 0x10000> &memory) {
  std::ifstream ifs;
  ifs.open(path);

  if (!ifs.is_open()) {
    throw NoSuchFileException(path);
  }

  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());

  if (content.size() <= 16 ||
      content[0] != 'N' || content[1] != 'E' || content[2] != 'S' ||
      content[3] != 0x1A) {
    throw InvalidRomException();
  }

  int prg_rom_size = content[4];
  int chr_rom_size = content[5];

  Flags6.raw = content[6];

  // Skip header
  int offset = 16;
  if (Flags6.TRAINER) {
    offset += 512;
  }
  std::copy(content.begin() + offset,
            content.begin() + offset + 16384 * prg_rom_size,
            memory.begin() + 0x8000);

}

}  // namespace nes
