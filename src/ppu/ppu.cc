#include "ppu.h"

#include <iostream>

#include "raylib.h"

#include "utils/assert.h"

namespace nes {

void PPU::Write(uint16_t addr, uint8_t v) {
  switch (addr) {
    case 0x2000: {
      PPUCTRL.raw = v;
      break;
    }
    case 0x2001: {
      PPUMASK.raw = v;
      break;
    }
    case 0x2003: {
      OAMADDR = v;
      break;
    }
    case 0x2005: {
      if (w == 1) {
        PPUSCROLL = v;
        y_scroll_ = PPUSCROLL;
        w = 0;
      } else {
        PPUSCROLL = v;
        x_scroll_ = PPUSCROLL;
        w = 1;
      }
      break;
    }
    case 0x2006: {
      if (w == 1) {
        PPUADDR = (PPUADDR << 8) | v;
        w = 0;
      } else {
        PPUADDR = v;
        w = 1;
      }
      break;
    }
    case 0x2007: {
      WriteVRAM(PPUADDR, v);
      if (PPUCTRL.VRAM_ADDR == 0) {
        PPUADDR += 1;
      } else {
        PPUADDR += 32;
      }
    }
    case 0x4014: {
      OAMDMA = v;
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
      return ret;
    }
    case 0x2007: {
      uint8_t ret = ReadVRAM(addr);

      if (PPUCTRL.VRAM_ADDR == 0) {
        PPUADDR += 1;
      } else {
        PPUADDR += 32;
      }

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
        // See https://www.nesdev.org/wiki/PPU_rendering#Visible_scanlines_(0-239)
        if (cycles_ % 8 == 0) {
          uint16_t base_addr = PPUCTRL.NAMETABLE_ADDR * 0x0400 + 0x2000;
          uint16_t addr = base_addr + cycles_ / 8 - 1 + scanline_ / 8 * 32;
          uint8_t tile_id = ReadVRAM(addr);

          uint16_t pattern_addr = PPUCTRL.BACKGROUND_PATTERN_ADDR * 0x1000;

          uint16_t tile_addr = tile_id * 16 + pattern_addr + scanline_ % 8;

          uint8_t plane0 = cartridge_.chr_rom[tile_addr];
          uint8_t plane1 = cartridge_.chr_rom[tile_addr + 8];

          const int kCellSize = 2;
          Color colors[] = {
            BLACK,
            WHITE,
            BLUE,
            RED,
          };

          for (int k = 7; k >= 0; --k) {
            uint8_t plane0_bit = (plane0 >> k) & 0x1;
            uint8_t plane1_bit = (plane1 >> k) & 0x1;
            uint8_t color_idx = plane0_bit + plane1_bit * 2;

            DrawRectangle(((cycles_ / 8) - 1) * kCellSize * 8 + (7 - k) * kCellSize,
                          scanline_ * kCellSize,
                          kCellSize, kCellSize, colors[color_idx]);
          }
        }
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
    } else {
      // TODO(yangsiyu):
      return;
      nes_assert(false, std::format("Unsupported vram write: {:#x}", addr));
    }
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 1) {
    nes_assert(false, std::format("Unsupported vram write for vertical mirroring: {:#x}", addr));
  } else {
    nes_assert(false, std::format("Unsupported vram write: {:#x}", addr));
  }
}

}  // namespace nes
