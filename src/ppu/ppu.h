#ifndef NES_EMULATOR_PPU_PPU_H_
#define NES_EMULATOR_PPU_PPU_H_

#include <cstdint>
#include <array>

#include "raylib.h"

#include "cpu/cpu.h"
#include "cartridge/cartridge.h"

namespace nes {

// See https://www.nesdev.org/wiki/PPU

class PPU {
 public:
  PPU(Cpu &cpu, Cartridge &cartridge)
      : cpu_(cpu),
        cartridge_(cartridge) {
    w = 0;
  }

  void Write(uint16_t addr, uint8_t value);
  uint8_t Read(uint16_t addr);

  void Tick();

  bool one_frame_finished() const { return one_frame_finished_; }
  const std::array<Color, 256 * 240> &pixels() const { return pixels_; }

  // These functions just for test.
  void TestRenderNametable(uint16_t addr);
  void TestRenderSprite();
  void TestPalettes();

 public:
  std::array<uint8_t, 256> OAM;

 private:
  uint8_t ReadVRAM(uint16_t addr);
  void WriteVRAM(uint16_t addr, uint8_t v);

  void IncrementHorizontalV();
  void IncrementVerticalV();

  uint8_t flip_h(uint8_t arg);

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

  // See https://www.nesdev.org/wiki/PPU_registers#Internal_registers
  // TODO(yangsiyu): Change this to portable
  union {
    struct {
      uint8_t COARSE_X : 5;
      uint8_t COARSE_Y : 5;
      uint8_t NAMETABLE : 2;
      uint8_t FINE_Y : 3;
    }__attribute__ ((packed));
    uint16_t raw;
  } v, t;
  uint8_t x;
  uint8_t w;

  // Sprites stuff
  // See https://www.nesdev.org/wiki/PPU_sprite_evaluation#References
  uint8_t n = 0;
  uint8_t m = 0;
  uint8_t oam_data_latch_;

  uint16_t bg_ls_shift;
  uint16_t bg_ms_shift;
  uint8_t attr_ls_shift;
  uint8_t attr_ms_shift;
  uint8_t attr_ls_latch;
  uint8_t attr_ms_latch;

  uint8_t read_buffer = 0;

  uint8_t tile_id;
  uint8_t attr;
  uint8_t bg_pattern_ls;
  uint8_t bg_pattern_ms;

  enum SpriteEvaluation {
    kLessEight = 0,
    kGreaterEight,
    kFail,
  } sprite_evaluation_state_ = kLessEight;

  struct Sprite {
    uint8_t tile_number;
    uint8_t pattern_ls_shift;
    uint8_t pattern_ms_shift;
    int x;
    int y;
    uint8_t palette;
    bool flip_h;
    bool flip_v;
    bool priority;  // false: front, true: back
  };

  std::array<Sprite, 8> sprites_;
  int sprites_count_ = 0;
  uint8_t sprites_idx_ = 0;
  uint16_t sprite_pattern_ls_shift_;
  uint16_t sprite_pattern_ms_shift_;

  uint8_t oam_size_ = 0;
  bool n_overflow_ = false;
  std::array<uint8_t, 4 * 8> oam_;
  std::array<uint8_t, 0x0800> vram_;
  std::array<uint8_t, 0x20> palettes_;

  std::array<Color, 256 * 240> pixels_;

  // I copied from https://bugzmanov.github.io/nes_ebook/chapter_6_3.html
  const std::array<Color, 0x40> kColors = {
    Color {0x62, 0x62, 0x62, 0xFF}, Color {0x0, 0x1f, 0xb2, 0xFF}, Color {0x24, 0x4, 0xc8, 0xFF}, Color {0x52, 0x0, 0xb2, 0xFF},
    Color {0x73, 0x0, 0x76, 0xFF}, Color {0x80, 0x0, 0x24, 0xFF}, Color {0x73, 0xb, 0x0, 0xFF}, Color {0x52, 0x28, 0x0, 0xFF},
    Color {0x24, 0x44, 0x0, 0xFF}, Color {0x0, 0x57, 0x0, 0xFF}, Color {0x0, 0x5c, 0x0, 0xFF}, Color {0x0, 0x53, 0x24, 0xFF},
    Color {0x0, 0x3c, 0x76, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF},
    Color {0xab, 0xab, 0xab, 0xFF}, Color {0xd, 0x57, 0xff, 0xFF}, Color {0x4b, 0x30, 0xff, 0xFF}, Color {0x8a, 0x13, 0xff, 0xFF},
    Color {0xbc, 0x8, 0xd6, 0xFF}, Color {0xd2, 0x12, 0x69, 0xFF}, Color {0xc7, 0x2e, 0x0, 0xFF}, Color {0x9d, 0x54, 0x0, 0xFF},
    Color {0x60, 0x7b, 0x0, 0xFF}, Color {0x20, 0x98, 0x0, 0xFF}, Color {0x0, 0xa3, 0x0, 0xFF}, Color {0x0, 0x99, 0x42, 0xFF},
    Color {0x0, 0x7d, 0xb4, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF},
    Color {0xff, 0xff, 0xff, 0xFF}, Color {0x53, 0xae, 0xff, 0xFF}, Color {0x90, 0x85, 0xff, 0xFF}, Color {0xd3, 0x65, 0xff, 0xFF},
    Color {0xff, 0x57, 0xff, 0xFF}, Color {0xff, 0x5d, 0xcf, 0xFF}, Color {0xff, 0x77, 0x57, 0xFF}, Color {0xfa, 0x9e, 0x0, 0xFF},
    Color {0xbd, 0xc7, 0x0, 0xFF}, Color {0x7a, 0xe7, 0x0, 0xFF}, Color {0x43, 0xf6, 0x11, 0xFF}, Color {0x26, 0xef, 0x7e, 0xFF},
    Color {0x2c, 0xd5, 0xf6, 0xFF}, Color {0x4e, 0x4e, 0x4e, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF},
    Color {0xff, 0xff, 0xff, 0xFF}, Color {0xb6, 0xe1, 0xff, 0xFF}, Color {0xce, 0xd1, 0xff, 0xFF}, Color {0xe9, 0xc3, 0xff, 0xFF},
    Color {0xff, 0xbc, 0xff, 0xFF}, Color {0xff, 0xbd, 0xf4, 0xFF}, Color {0xff, 0xc6, 0xc3, 0xFF}, Color {0xff, 0xd5, 0x9a, 0xFF},
    Color {0xe9, 0xe6, 0x81, 0xFF}, Color {0xce, 0xf4, 0x81, 0xFF}, Color {0xb6, 0xfb, 0x9a, 0xFF}, Color {0xa9, 0xfa, 0xc3, 0xFF},
    Color {0xa9, 0xf0, 0xf4, 0xFF}, Color {0xb8, 0xb8, 0xb8, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}, Color {0x0, 0x0, 0x0, 0xFF}
  };

  const int kScanLine = 261;
  const int kCycles = 340;

  bool one_frame_finished_;

  int scanline_ = 0;
  int cycles_ = 0;

  Cpu &cpu_;
  Cartridge &cartridge_;
};

}  // namespace nes

#endif  // NES_EMULATOR_PPU_PPU_H_
