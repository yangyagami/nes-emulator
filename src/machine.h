#include <array>
#include <string>

#include "bus/bus.h"
#include "cpu/cpu.h"
#include "ppu/ppu.h"
#include "cartridge/cartridge.h"

namespace nes {

class Machine {
 public:
  Machine(const std::string &path);

  int Run();

 private:
  std::array<uint8_t, 0x0800> memory_;
  Cartridge cartridge_;
  Bus bus_;
  Cpu cpu_;
  PPU ppu_;

  std::string rom_path_;
};

}  // namespace nes
