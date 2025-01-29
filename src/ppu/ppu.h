#ifndef NES_EMULATOR_PPU_PPU_H_
#define NES_EMULATOR_PPU_PPU_H_

#include <cstdint>

namespace nes {

struct PPU {
  // See https://www.nesdev.org/wiki/PPU_registers#PPUCTRL
  union {
    struct {
      uint8_t NAMETABLE_ADDR : 2;
      uint8_t VRAM_ADDR : 1;
      uint8_t SPRITE_PATTERN_ADDR : 1;
      uint8_t BACKGROUND_PATTERN_ADDR : 1;
      uint8_t SPRITE_SIZE : 1;
      uint8_t PPU_MASTER : 1;
      uint8_t VBLANK_NMI : 1;
    };
    uint8_t raw;
  } PPUCTRL;

  void Write(uint16_t addr, uint8_t v);

  void Tick();
};

}  // namespace nes

#endif  // NES_EMULATOR_PPU_PPU_H_
