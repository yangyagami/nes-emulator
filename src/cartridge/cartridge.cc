#include "cartridge.h"

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <cstdint>

namespace nes {

Cartridge::Cartridge(const std::string &path) {
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

  prg_rom.resize(prg_rom_size * 16384);
  chr_rom.resize(chr_rom_size * 8192);

  std::copy(content.begin() + offset,
            content.begin() + offset + 16384 * prg_rom_size,
            prg_rom.begin());

  offset += 16384 * prg_rom_size;

  std::copy(content.begin() + offset,
            content.begin() + offset + 8192 * chr_rom_size,
            chr_rom.begin());

}

}  // namespace nes
