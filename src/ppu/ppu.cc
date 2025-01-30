#include "ppu.h"

#include <iostream>

#include "utils/assert.h"

namespace nes {

void PPU::Write(uint16_t addr, uint8_t v) {
  switch (addr) {
    case 0x2000: {
      PPUCTRL.raw = v;
      std::cout << std::format("PPUCTRL: {:#x}", PPUCTRL.raw) << std::endl;
      break;
    }
    case 0x2001: {
      PPUMASK.raw = v;
      std::cout << std::format("PPUMASK: {:#x}", PPUMASK.raw) << std::endl;
      break;
    }
    case 0x2003: {
      OAMADDR = v;
      break;
    }
    case 0x2005: {
      std::cout << std::format("PPUSCROLL: {:#x}, w: {}", PPUSCROLL, w) << std::endl;
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
      std::cout << std::format("PPUADDR: {:#x}, w: {}", PPUADDR, w) << std::endl;
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
      std::cout << std::format("PPUADDR: {:#x} PPUDATA: {:#x}", PPUADDR, v) << std::endl;
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
    if (PPUCTRL.VBLANK_NMI) {
      cpu_.nmi_flipflop = true;
    }
    PPUSTATUS.VBLANK = 1;
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

}  // namespace nes
