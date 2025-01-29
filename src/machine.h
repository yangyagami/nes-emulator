#include <array>

#include "bus/bus.h"
#include "cpu/cpu.h"
#include "cartridge/cartridge.h"

namespace nes {

class Machine {
 public:
  Machine();

  void Run();

 private:
  std::array<uint8_t, 0x0800> memory_;
  Cartridge cartridge_;
  Bus bus_;
  Cpu cpu_;
};

}  // namespace nes
