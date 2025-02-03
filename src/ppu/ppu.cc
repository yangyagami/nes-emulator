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
        t.COARSE_Y = value >> 3;
        t.FINE_Y = value & 0x07;
        w = 0;
      } else {
        t.COARSE_X = value >> 3;
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

  if (PPUMASK.BACKGROUND_RENDERING && scanline_ >= -1 && scanline_ <= 239) {
    if (cycles_ >= 0 && cycles_ <= 256 || cycles_ >= 321 || cycles_ <= 336) {
      switch (cycles_ % 8) {
        case 2: {  // Nametable
          uint16_t nm_addr = 0x2000 | (v.raw & 0x0FFF);
          tile_id = ReadVRAM(nm_addr);
          break;
        }
        case 4: {  // Attribute
          uint16_t attr_addr =
              0x23C0 | (v.raw & 0x0C00) | ((v.raw >> 4) & 0x38) | ((v.raw >> 2) & 0x07);
          attr = ReadVRAM(attr_addr);
          break;
        }
        case 6: {  // BG lsbits
          bg_pattern_ls =
              ReadVRAM(PPUCTRL.BACKGROUND_PATTERN_ADDR * 0x1000 + tile_id * 16 + v.FINE_Y);
          break;
        }
        case 7: {  // BG msbits && IncrementHorizontalV
          bg_pattern_ms = ReadVRAM(
              PPUCTRL.BACKGROUND_PATTERN_ADDR * 0x1000 + tile_id * 16 + v.FINE_Y + 8);
          break;
        }
      }
    }

    if (cycles_ >= 328 || cycles_ <= 256) {
      if (cycles_ % 8 == 0 && cycles_ / 8 > 0) {
        bg_ls_shift = (bg_ls_shift & 0xFF00) | bg_pattern_ls;
        bg_ms_shift = (bg_ms_shift & 0xFF00) | bg_pattern_ms;
        IncrementHorizontalV();
      }

      if (cycles_ == 256) {
        IncrementVerticalV();
      }
    }

    if (cycles_ == 257) {
      v.COARSE_X = t.COARSE_X;
      uint8_t mask = 0x01;
      v.NAMETABLE = (v.NAMETABLE & ~mask) | (t.NAMETABLE & mask);
    }

    if (scanline_ == -1) {
      if (cycles_ >= 280 && cycles_ <= 304) {
        v.COARSE_Y = t.COARSE_Y;
        uint8_t mask = 0x2;
        v.NAMETABLE = (v.NAMETABLE & ~mask) | (t.NAMETABLE & mask);
        v.FINE_Y = t.FINE_Y;
      }
    }
  }

  // Pre scanline
  if (scanline_ == -1 || scanline_ == kScanLine) {
    // TODO(yangsiyu):
    if (cycles_ == 1) {
      PPUSTATUS.VBLANK = 0;
    }
  }

  // VBLANK
  if (scanline_ == 241 && cycles_ == 1) {
    PPUSTATUS.VBLANK = 1;
    if (PPUCTRL.VBLANK_NMI) {
      cpu_.nmi_flipflop = true;
    }
  }

  if (scanline_ >= 0 && scanline_ <= 239) {
    if (cycles_ >= 1 && cycles_ <= 256) {
      if (PPUMASK.BACKGROUND) {
        uint16_t mask = (0x8000 >> x);
        uint8_t plane0 = (bg_ls_shift & mask) > 0;
        uint8_t plane1 = (bg_ms_shift & mask) > 0;

        const int kCellSize = 2;

        Color colors[] = {
          BLACK,
          BLUE,
          BLACK,
          WHITE,
        };

        bg_ls_shift <<= 1;
        bg_ms_shift <<= 1;

        uint8_t colorid = plane0 + plane1 * 2;

        DrawRectangle(cycles_ * kCellSize,
                      scanline_ * kCellSize,
                      kCellSize, kCellSize, colors[colorid]);

        // See https://www.nesdev.org/wiki/PPU_rendering#Visible_scanlines_(0-239)
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
    int x = (i - addr) % 32;
    int y = (i - addr) / 32;
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
  if (addr <= 0x1FFF) {
    // PATTERN TABLE
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

void PPU::IncrementHorizontalV() {
  if ((v.raw & 0x001F) == 31) {  // if coarse X == 31
    v.raw &= ~0x001F;            // coarse X = 0
    v.raw ^= 0x0400;             // switch horizontal nametable
  } else {
    v.raw += 1;                  // increment coarse X
  }
}

void PPU::IncrementVerticalV() {
  if ((v.raw & 0x7000) != 0x7000) {             // if fine Y < 7
    v.raw += 0x1000;                            // increment fine Y
  } else {
    v.raw &= ~0x7000;                           // fine Y = 0

    int y = (v.raw & 0x03E0) >> 5;              // let y = coarse Y

    if (y == 29) {
      y = 0;                                    // coarse Y = 0
      v.raw ^= 0x0800;                          // switch vertical nametable
    } else if (y == 31) {
      y = 0;                                    // coarse Y = 0, nametable not switched
    } else {
      y += 1;                                   // increment coarse Y
    }
    v.raw = (v.raw & ~0x03E0) | (y << 5);     // put coarse Y back into v
  }
}

}  // namespace nes
