#ifndef NES_EMULATOR_PPU_PPU_H_
#define NES_EMULATOR_PPU_PPU_H_

#include <cstdint>

#include "cpu/cpu.h"

namespace nes {

class PPU {
 public:
  PPU(Cpu &cpu) : cpu_(cpu) {
    w = 0;
  }

  void Write(uint16_t addr, uint8_t v);
  uint8_t Read(uint16_t addr);

  void Tick();

  bool one_frame_finished() const { return one_frame_finished_; }

 private:
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

  uint8_t OAMADDR;
  uint8_t PPUSCROLL;
  uint16_t PPUADDR;
  uint8_t OAMDMA;

  uint8_t x_scroll_;
  uint8_t y_scroll_;

  // See https://www.nesdev.org/wiki/PPU_registers#Internal_registers
  uint8_t w;

  const int kScanLine = 261;
  const int kCycles = 340;

  bool one_frame_finished_;

  int scanline_ = -1;
  int cycles_ = 0;

  Cpu &cpu_;
};

}  // namespace nes

#endif  // NES_EMULATOR_PPU_PPU_H_
