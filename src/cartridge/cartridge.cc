#include "cartridge.h"

#include <array>
#include <string>
#include <fstream>

Cartridge::Cartridge(const std::string &path, std::array<uint8_t, 0x10000> &memory) {
  std::ifstream ifs;
  ifs.open(path);

  if (!ifs.is_open()) {
    // TODO(yangsiyu): Error
  }

  std::string content((std::istreambuf_iterator<char>(inputFile)),
                      std::istreambuf_iterator<char>());

  if (content.size() <= 16 ||
      content[0] != 'N' || content[1] != 'E' || content[2] != 'S' ||
      content[3] != 0x1A) {
    // TODO(yangsiyu): Error (Invalid Rom format)
  }

  int prg_rom_size = content[4];
  int chr_rom_size = content[5];
}
