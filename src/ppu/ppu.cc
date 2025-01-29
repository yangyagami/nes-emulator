#include "ppu.h"

#include "utils/assert.h"

namespace nes {

void PPU::Write(uint16_t addr, uint8_t v) {
  switch (addr) {
    case 0x2000: {
      PPUCTRL.raw = v;
      break;
    }
    default: {
      nes_assert(false, std::format("Unsupported ppu write: {:#x}\n", addr));
      break;
    }
  }
}

void PPU::Tick() {
}

}  // namespace nes
