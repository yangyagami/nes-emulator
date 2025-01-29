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

  // See https://www.nesdev.org/wiki/PPU_registers#PPUMASK
  union {
    struct {
      uint8_t GREYSCALE : 1;
      uint8_t BACKGROUND : 1;
      uint8_t SPRITES : 1;
      uint8_t BACKGROUND_RENDERING : 1;
      uint8_t SPRITE_RENDERING : 1;
    };
    uint8_t raw;
  } PPUMASK;

  // See https://www.nesdev.org/wiki/PPU_registers#PPUSTATUS
  union {
    struct {
      uint8_t OPEN_BUS : 5;
      uint8_t SPRITE_OVERFLOW : 1;
      uint8_t SPRITE_HIT : 1;
      uint8_t VBLANK : 1;
    };
    uint8_t raw;
  } PPUSTATUS;

  void Write(uint16_t addr, uint8_t v);

  void Tick();

 private:
  const int kScanLine = 261;
  const int kCycles = 340;

  int scanline_ = -1;
  int cycles_ = 0;
};

}  // namespace nes

#endif  // NES_EMULATOR_PPU_PPU_H_
