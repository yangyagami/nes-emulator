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
    Color {0x80, 0x80, 0x80, 0xFF}, Color {0x00, 0x3D, 0xA6, 0xFF}, Color {0x00, 0x12, 0xB0, 0xFF}, Color {0x44, 0x00, 0x96, 0xFF}, Color {0xA1, 0x00, 0x5E, 0xFF},
    Color {0xC7, 0x00, 0x28, 0xFF}, Color {0xBA, 0x06, 0x00, 0xFF}, Color {0x8C, 0x17, 0x00, 0xFF}, Color {0x5C, 0x2F, 0x00, 0xFF}, Color {0x10, 0x45, 0x00, 0xFF},
    Color {0x05, 0x4A, 0x00, 0xFF}, Color {0x00, 0x47, 0x2E, 0xFF}, Color {0x00, 0x41, 0x66, 0xFF}, Color {0x00, 0x00, 0x00, 0xFF}, Color {0x05, 0x05, 0x05, 0xFF},
    Color {0x05, 0x05, 0x05, 0xFF}, Color {0xC7, 0xC7, 0xC7, 0xFF}, Color {0x00, 0x77, 0xFF, 0xFF}, Color {0x21, 0x55, 0xFF, 0xFF}, Color {0x82, 0x37, 0xFA, 0xFF},
    Color {0xEB, 0x2F, 0xB5, 0xFF}, Color {0xFF, 0x29, 0x50, 0xFF}, Color {0xFF, 0x22, 0x00, 0xFF}, Color {0xD6, 0x32, 0x00, 0xFF}, Color {0xC4, 0x62, 0x00, 0xFF},
    Color {0x35, 0x80, 0x00, 0xFF}, Color {0x05, 0x8F, 0x00, 0xFF}, Color {0x00, 0x8A, 0x55, 0xFF}, Color {0x00, 0x99, 0xCC, 0xFF}, Color {0x21, 0x21, 0x21, 0xFF},
    Color {0x09, 0x09, 0x09, 0xFF}, Color {0x09, 0x09, 0x09, 0xFF}, Color {0xFF, 0xFF, 0xFF, 0xFF}, Color {0x0F, 0xD7, 0xFF, 0xFF}, Color {0x69, 0xA2, 0xFF, 0xFF},
    Color {0xD4, 0x80, 0xFF, 0xFF}, Color {0xFF, 0x45, 0xF3, 0xFF}, Color {0xFF, 0x61, 0x8B, 0xFF}, Color {0xFF, 0x88, 0x33, 0xFF}, Color {0xFF, 0x9C, 0x12, 0xFF},
    Color {0xFA, 0xBC, 0x20, 0xFF}, Color {0x9F, 0xE3, 0x0E, 0xFF}, Color {0x2B, 0xF0, 0x35, 0xFF}, Color {0x0C, 0xF0, 0xA4, 0xFF}, Color {0x05, 0xFB, 0xFF, 0xFF},
    Color {0x5E, 0x5E, 0x5E, 0xFF}, Color {0x0D, 0x0D, 0x0D, 0xFF}, Color {0x0D, 0x0D, 0x0D, 0xFF}, Color {0xFF, 0xFF, 0xFF, 0xFF}, Color {0xA6, 0xFC, 0xFF, 0xFF},
    Color {0xB3, 0xEC, 0xFF, 0xFF}, Color {0xDA, 0xAB, 0xEB, 0xFF}, Color {0xFF, 0xA8, 0xF9, 0xFF}, Color {0xFF, 0xAB, 0xB3, 0xFF}, Color {0xFF, 0xD2, 0xB0, 0xFF},
    Color {0xFF, 0xEF, 0xA6, 0xFF}, Color {0xFF, 0xF7, 0x9C, 0xFF}, Color {0xD7, 0xE8, 0x95, 0xFF}, Color {0xA6, 0xED, 0xAF, 0xFF}, Color {0xA2, 0xF2, 0xDA, 0xFF},
    Color {0x99, 0xFF, 0xFC, 0xFF}, Color {0xDD, 0xDD, 0xDD, 0xFF}, Color {0x11, 0x11, 0x11, 0xFF}, Color {0x11, 0x11, 0x11, 0xFF}
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
