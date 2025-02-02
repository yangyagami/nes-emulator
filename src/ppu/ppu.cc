#include "ppu.h"

#include <iostream>

#include "raylib.h"

#include "utils/assert.h"

namespace nes {

void PPU::Write(uint16_t addr, uint8_t value) {
  switch (addr) {
    case 0x2000: {
      // See https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
      PPUCTRL.raw = value;
      t.NAMETABLE = PPUCTRL.NAMETABLE_ADDR;
      break;
    }
    case 0x2001: {
      PPUMASK.raw = value;
      break;
    }
    case 0x2003: {
      OAMADDR = value;
      break;
    }
    case 0x2005: {
      if (w == 1) {
        t.COARSE_Y = (value & 0x3E0) >> 5;
        t.FINE_Y = value & 0x07;
        w = 0;
      } else {
        t.COARSE_X = (value & 0xF8) >> 3;
        x = value & 0x7;
        w = 1;
      }
      break;
    }
    case 0x2006: {
      if (w == 1) {
        t.raw |= value;
        v.raw = t.raw;
        w = 0;
      } else {
        // Clear first, then write.
        t.raw = 0x0;
        uint16_t mask = 0x3f00;
        value &= 0x3f;
        int tmp = value << 8;
        t.raw = (t.raw & ~mask) | (tmp & mask);
        w = 1;
      }
      break;
    }
    case 0x2007: {
      WriteVRAM(v.raw, value);
      v.raw += (PPUCTRL.VRAM_ADDR == 0 ? 1 : 32);
      break;
    }
    case 0x4014: {
      OAMDMA = value;
      break;
    }
    default: {
      nes_assert(false, std::format("Unsupported ppu write: {:#x}\n", addr));
      break;
    }
  }
}

uint8_t PPU::Read(uint16_t addr) {
  switch (addr) {
    case 0x2002: {
      uint8_t ret = PPUSTATUS.raw;
      PPUSTATUS.VBLANK = 0;
      w = 0;
      return ret;
    }
    case 0x2007: {
      uint8_t ret = ReadVRAM(v.raw);
      v.raw += (PPUCTRL.VRAM_ADDR == 0 ? 1 : 32);
      return ret;
    }
    default: {
      nes_assert(false, std::format("Unsupported ppu read: {:#x}\n", addr));
    }
  }
}

void PPU::Tick() {
  one_frame_finished_ = false;

  if (scanline_ == -1 || scanline_ == kScanLine) {
    // TODO(yangsiyu):
    if (cycles_ == 1) {
      PPUSTATUS.VBLANK = 0;
    }
    if (cycles_ == 304) {
      v.COARSE_Y = t.COARSE_Y;
      v.NAMETABLE = (v.NAMETABLE & 0x01) | (t.NAMETABLE & 0x02);
      v.FINE_Y = t.FINE_Y;
    }
  }
  if (scanline_ == 241 && cycles_ == 1) {
    PPUSTATUS.VBLANK = 1;
    if (PPUCTRL.VBLANK_NMI) {
      cpu_.nmi_flipflop = true;
    }
  }

  if (scanline_ >= 0 && scanline_ <= 239) {
    if (cycles_ >= 1 && cycles_ <= 256) {
      if (PPUMASK.BACKGROUND) {
        // Picked from https://www.nesdev.org/wiki/PPU_scrolling#Coarse_X_increment
        x++;
        if (x > 7) {
          x = 0;
          if ((v.raw & 0x001F) == 31) {// if coarse X == 31
            v.raw &= ~0x001F;          // coarse X = 0
            v.raw ^= 0x0400;           // switch horizontal nametable
          } else {
            v.raw += 1;                // increment coarse X
          }
        }
        if (cycles_ == 256) {
          if ((v.raw & 0x7000) != 0x7000) {        // if fine Y < 7
            v.raw += 0x1000;                      // increment fine Y
          } else {
            v.raw &= ~0x7000;                     // fine Y = 0

            int y = (v.raw & 0x03E0) >> 5;        // let y = coarse Y

            if (y == 29) {
              y = 0;                          // coarse Y = 0
              v.raw ^= 0x0800;                    // switch vertical nametable
            } else if (y == 31) {
              y = 0;                          // coarse Y = 0, nametable not switched
            } else {
              y += 1;                         // increment coarse Y
              v.raw = (v.raw & ~0x03E0) | (y << 5);     // put coarse Y back into v
            }
          }
        }

        if (cycles_ % 8 == 0) {
          uint16_t nm_address = 0x2000 | (v.raw & 0x0FFF);
          uint16_t attr_address = 0x23C0 | (v.raw & 0x0C00) | ((v.raw >> 4) & 0x38) | ((v.raw >> 2) & 0x07);

          uint8_t tile_id = ReadVRAM(nm_address);
          uint8_t attr_id = ReadVRAM(attr_address);

          uint16_t pattern_addr = PPUCTRL.BACKGROUND_PATTERN_ADDR * 0x1000;

          uint16_t tile_addr = tile_id * 16 + pattern_addr + scanline_ % 8;

          uint8_t plane0 = cartridge_.chr_rom[tile_addr];
          uint8_t plane1 = cartridge_.chr_rom[tile_addr + 8];

          const int kCellSize = 2; // TODO(yangsiyu): Fit to window.

          Color colors[] = {
            BLACK,
            BLUE,
            RED,
            WHITE,
          };

          for (int k = 7; k >= 0; --k) {
            uint8_t plane0_bit = (plane0 >> k) & 0x1;
            uint8_t plane1_bit = (plane1 >> k) & 0x1;
            uint8_t color_idx = plane0_bit + plane1_bit * 2;

            // TODO(yangsiyu): Make this to a single frame data.

            DrawRectangle((cycles_ - 1) / 8 * kCellSize * 8 + (7 - k) * kCellSize,
                          scanline_ * kCellSize,
                          kCellSize, kCellSize, colors[color_idx]);
          }
        }


        // See https://www.nesdev.org/wiki/PPU_rendering#Visible_scanlines_(0-239)
        // if (cycles_ % 8 == 0) {
        //   uint16_t base_addr = PPUCTRL.NAMETABLE_ADDR * 0x0400 + 0x2000;

        //   uint16_t attr_addr = base_addr + 0x03C0 + (cycles_ - 1) / 32 + scanline_ / 32 * 8;
        //   uint8_t attr = ReadVRAM(attr_addr);

        //   uint8_t palette_idx = 1;

        //   int column = (cycles_ - 1) / 8;
        //   int row = scanline_ / 8;

        //   if (column % 4 / 2 == 0) {
        //     // Left
        //     if (row % 4 / 2 == 0) {
        //       // Top left
        //       palette_idx = (attr & 0x3) * 4;
        //     } else {
        //       // Bottom left
        //       palette_idx = ((attr & 0x30) >> 4) * 4;
        //     }
        //   } else {
        //     // Right
        //     if (row % 4 / 2 == 0) {
        //       // Top right
        //       palette_idx = ((attr & 0xC) >> 2) * 4;
        //     } else {
        //       // Bottom right
        //       palette_idx = ((attr & 0xC0) >> 6) * 4;
        //     }
        //   }

        //   uint16_t nm_addr = base_addr + (cycles_ - 1) / 8 + scanline_ / 8 * 32;
        //   uint8_t tile_id = ReadVRAM(nm_addr);

        //   uint16_t pattern_addr = PPUCTRL.BACKGROUND_PATTERN_ADDR * 0x1000;

        //   uint16_t tile_addr = tile_id * 16 + pattern_addr + scanline_ % 8;

        //   uint8_t plane0 = cartridge_.chr_rom[tile_addr];
        //   uint8_t plane1 = cartridge_.chr_rom[tile_addr + 8];

        //   const int kCellSize = 2; // TODO(yangsiyu): Fit to window.

        //   for (int k = 7; k >= 0; --k) {
        //     uint8_t plane0_bit = (plane0 >> k) & 0x1;
        //     uint8_t plane1_bit = (plane1 >> k) & 0x1;
        //     uint8_t color_idx = plane0_bit + plane1_bit * 2;

        //     // TODO(yangsiyu): Make this to a single frame data.

        //     uint8_t colorid = ReadVRAM(0x3F00 + palette_idx + color_idx);
        //     DrawRectangle((cycles_ - 1) / 8 * kCellSize * 8 + (7 - k) * kCellSize,
        //                   scanline_ * kCellSize,
        //                   kCellSize, kCellSize, kColors[colorid]);
        //   }
        // }
      }
    }
    if (cycles_ == 257) {
      if (PPUMASK.BACKGROUND) {
        v.COARSE_X = t.COARSE_X;
        v.NAMETABLE = t.NAMETABLE & 0x1;
      }
    }
  }

  cycles_++;
  if (cycles_ > kCycles) {
    scanline_++;
    if (scanline_ > kScanLine) {
      scanline_ = -1;
      one_frame_finished_ = true;
    }
    cycles_ = 0;
  }
}

void PPU::TestRenderNametable(uint16_t addr) {
  const int kCellSize = 2;
  Color colors[] = {
    BLACK,
    WHITE,
    BLUE,
    RED,
  };
  for (int i = addr; i < addr + 0x0400; i++) {
    uint8_t tile_id = ReadVRAM(i);
    int x = (i - 0x2000) % 32;
    int y = (i - 0x2000) / 32;
    for (int j = tile_id * 16; j < tile_id * 16 + 8; ++j) {
      uint8_t plane0 = cartridge_.chr_rom[j];
      uint8_t plane1 = cartridge_.chr_rom[j + 8];

      for (int k = 7; k >= 0; --k) {
        uint8_t plane0_bit = (plane0 >> k) & 0x1;
        uint8_t plane1_bit = (plane1 >> k) & 0x1;
        uint8_t color_idx = plane0_bit + plane1_bit * 2;

        DrawRectangle(x * 8 * kCellSize + (7 - k) * kCellSize,
                      y * 8 * kCellSize + (j - tile_id * 16) * kCellSize,
                      kCellSize, kCellSize, colors[color_idx]);
      }
    }
  }
}

uint8_t PPU::ReadVRAM(uint16_t addr) {
  if (addr <= 0x0FFF) {
    // PATTERN TABLE 0
    return cartridge_.chr_rom[addr];
  } else if (addr >= 0x2000 && addr <= 0x23FF) {
    return vram_[addr - 0x2000];
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 0) {
    if (addr >= 0x2400 && addr <= 0x27FF) {
      return vram_[addr - 0x2400];
    } else if (addr >= 0x2800 && addr <= 0x2BFF) {
      return vram_[addr - 0x2400];
    } else if (addr >= 0x2C00 && addr <= 0x2FFF) {
      return vram_[addr - 0x2800];
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
      return palettes_[addr - 0x3F00];
    } else {
      nes_assert(false, std::format("Unsupported vram read: {:#x}", addr));
    }
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 1) {
    nes_assert(false, std::format("Unsupported vram read for vertical mirroring: {:#x}", addr));
  } else {
    nes_assert(false, std::format("Unsupported vram read: {:#x}", addr));
  }
}

void PPU::WriteVRAM(uint16_t addr, uint8_t v) {
  if (addr >= 0x2000 && addr <= 0x23FF) {
    vram_[addr - 0x2000] = v;
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 0) {
    if (addr >= 0x2400 && addr <= 0x27FF) {
      vram_[addr - 0x2400] = v;
    } else if (addr >= 0x2800 && addr <= 0x2BFF) {
      vram_[addr - 0x2400] = v;
    } else if (addr >= 0x2C00 && addr <= 0x2FFF) {
      vram_[addr - 0x2800] = v;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
      palettes_[addr - 0x3F00] = v;
    } else {
      nes_assert(false, std::format("Unsupported vram write: {:#x}", addr));
    }
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 1) {
    nes_assert(false, std::format("Unsupported vram write for vertical mirroring: {:#x}", addr));
  } else {
    nes_assert(false, std::format("Unsupported vram write: {:#x}", addr));
  }
}

}  // namespace nes
