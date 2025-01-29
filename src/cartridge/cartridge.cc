#include "cartridge.h"

#include <format>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>

namespace nes {

bool Cartridge::LoadRomFile(const std::string &path) {
  std::ifstream ifs;
  ifs.open(path);

  if (!ifs.is_open()) {
    std::cerr << std::format("No such file: {}\n", path);
    return false;
  }

  prg_rom.clear();
  chr_rom.clear();

  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());

  if (content.size() <= 16 ||
      content[0] != 'N' || content[1] != 'E' || content[2] != 'S' ||
      content[3] != 0x1A) {
    std::cerr << std::format("Invalid rom format\n");
    return false;
  }

  int prg_rom_size = content[4];
  int chr_rom_size = content[5];

  Flags6.raw = content[6];
  Flags7.raw = content[7];
  mapper = (Flags7.MAPPER_NUMBER << 8) | Flags6.MAPPER_NUMBER;

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

  return true;
}

}  // namespace nes
