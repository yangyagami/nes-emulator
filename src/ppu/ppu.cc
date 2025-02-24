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
    case 0x2004: {
      OAM[OAMADDR++] = value;
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
    case 0x2005: {
      return OAM[OAMADDR];
    }
    case 0x2007: {
      uint8_t ret = read_buffer;
      read_buffer = ReadVRAM(v.raw);
      v.raw += (PPUCTRL.VRAM_ADDR == 0 ? 1 : 32);
      return ret;
    }
    default: {
      // nes_assert(false, std::format("Unsupported ppu read: {:#x}\n", addr));
      std::cout << std::format("Warn: unsupported ppu read: {:#x}\n", addr);
      return 0;
    }
  }
}

void PPU::Tick() {
  /*
    Picked from nesdev:
    The PPU renders 262 scanlines per frame.
    Each scanline lasts for 341 PPU clock cycles (113.667 CPU clock cycles; 1 CPU cycle = 3 PPU cycles),
    with each clock cycle producing one pixel.
    The line numbers given here correspond to how the internal PPU frame counters count lines.
   */
  one_frame_finished_ = false;

  // Background tile loaded
  if (PPUMASK.BACKGROUND_RENDERING &&
      ((scanline_ >= 0 && scanline_ <= 239) || scanline_ == kScanLine)) {
    // See https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
    if ((cycles_ >= 1 && cycles_ <= 256) || (cycles_ >= 321 && cycles_ <= 336)) {
      // Shift registers stuff.
      // Transfer to high 8 bit.
      bg_ls_shift <<= 1;
      bg_ms_shift <<= 1;

      attr_ls_shift <<= 1;
      attr_ms_shift <<= 1;
      attr_ls_shift |= attr_ls_latch;
      attr_ms_shift |= attr_ms_latch;

      switch (cycles_ % 8) {
        case 2: {  // Nametable
          uint16_t nm_addr = 0x2000 | (v.raw & 0x0FFF);
          tile_id = ReadVRAM(nm_addr);
          break;
        }
        case 4: {  // Attribute
          /*
            (v.raw & 0x0C00) pick selected nametable.
            (v.raw >> 4) & 0x38 pick 8 bit, that contains coarse y.
            (v.raw >> 2) & 0x07 pick 8 bit, that contains coarse x.
          */
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
        case 7: {  // BG msbits
          bg_pattern_ms = ReadVRAM(
              PPUCTRL.BACKGROUND_PATTERN_ADDR * 0x1000 + tile_id * 16 + v.FINE_Y + 8);
          break;
        }
      }
    }

    if (cycles_ >= 328 || cycles_ <= 256) {
      if (cycles_ % 8 == 0 && cycles_ / 8 > 0) {  // cycles 0 idle on background.
        bg_ls_shift = (bg_ls_shift & 0xFF00) | bg_pattern_ls;
        bg_ms_shift = (bg_ms_shift & 0xFF00) | bg_pattern_ms;

        int coarse_x = (v.COARSE_X >> 1) & 0x1;
        int coarse_y = (v.COARSE_Y >> 1) & 0x1;

        int offset = coarse_y * 4 + coarse_x * 2;
        attr_ls_latch = (attr >> offset) & 0x1;
        attr_ms_latch = (attr >> (offset + 1)) & 0x1;

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

    if (cycles_ >= 280 && cycles_ <= 304 && scanline_ == kScanLine) {
      v.COARSE_Y = t.COARSE_Y;
      uint8_t mask = 0x2;
      v.NAMETABLE = (v.NAMETABLE & ~mask) | (t.NAMETABLE & mask);
      v.FINE_Y = t.FINE_Y;
    }
  }

  // Sprite evaluation
  // See https://www.nesdev.org/wiki/PPU_sprite_evaluation#References
  if (PPUMASK.SPRITE_RENDERING) {
    if (scanline_ >= 0 && scanline_ <= 239) {
      if (cycles_ >= 1 && cycles_ <= 64) {
        if (cycles_ % 2 == 0) {
          oam_[cycles_ / 2 - 1] = 0xFF;
        }
      } else if (cycles_ >= 65 && cycles_ <= 256) {
        if (cycles_ % 2 != 0) { // odd cycles
          // On odd cycles, data is read from (primary) OAM
          oam_data_latch_ = OAM[n * 4 + m];
        } else { // even cycles
          // On even cycles, data is written to secondary OAM
          // (unless secondary OAM is full, in which case it will read the value in secondary OAM instead)
          switch (sprite_evaluation_state_) {
            case kLessEight: {
              if (m == 0) {
                // Check y-coord whether in range.
                if (scanline_ >= oam_data_latch_ &&
                    scanline_ <= oam_data_latch_ + 7) {
                  oam_size_++;
                  if (oam_size_ > 8) {
                    oam_size_ = 8;
                    sprite_evaluation_state_ = kGreaterEight;
                  } else {
                    oam_[(oam_size_ - 1) * 4 + m] = oam_data_latch_;
                    m++;
                  }
                } else {
                  m = 0;
                  n++;

                  if (n >= 64) {
                    sprite_evaluation_state_ = kFail;
                  }
                }
              } else {
                nes_assert(oam_size_ != 0, "Invalid oam_size_!!!");

                oam_[(oam_size_ - 1) * 4 + m] = oam_data_latch_;
                m++;
                if (m >= 4) {
                  m = 0;
                  n++;
                  if (n >= 64) {
                    sprite_evaluation_state_ = kFail;
                  }
                }
              }
              break;
            }
            case kGreaterEight: {
              // We don't read oam.
              if (m == 0) {
                // Check y-coord whether in range.
                if (scanline_ >= oam_data_latch_ &&
                    scanline_ <= oam_data_latch_ + 7) {
                  PPUSTATUS.SPRITE_OVERFLOW = 1;
                  m++;
                } else {
                  n++;
                  m++;

                  // Make sure the m not leaked
                  if (m >= 4) {
                    m = 0;
                  }

                  if (n >= 64) {
                    sprite_evaluation_state_ = kFail;
                    n = 0;
                    m = 0;
                  }
                }
              } else {
                m++;
                if (m >= 4) {
                  m = 0;
                  n++;
                }
              }
              break;
            }
            case kFail: {
              // We just do nothing here.
              break;
            }
          }
        }
      } else if (cycles_ >= 257 && cycles_ <= 320) {
        if (cycles_ == 257) {
          sprites_idx_ = 0;
          sprites_count_ = oam_size_;
        }

        if (sprites_idx_ < sprites_count_) {
          int tmp = cycles_ % 8;

          switch (cycles_ % 8) {
            case 0: {
              sprites_idx_++;
              break;
            }
            case 1: {
              sprites_[sprites_idx_].y = oam_[sprites_idx_ * 4 + (tmp - 1)];
              break;
            }
            case 2: {
              // Tile number
              sprites_[sprites_idx_].tile_number = oam_[sprites_idx_ * 4 + (tmp - 1)];
              break;
            }
            case 3: {
              // Attributes
              uint8_t attr = oam_[sprites_idx_ * 4 + (tmp - 1)];
              sprites_[sprites_idx_].palette = (attr & 0x3);
              sprites_[sprites_idx_].flip_h = (attr & 0x40) > 0;
              sprites_[sprites_idx_].flip_v = (attr & 0x80) > 0;
              sprites_[sprites_idx_].priority = (attr & 0x20) > 0;

              if (PPUCTRL.SPRITE_SIZE == 0) {
                // 8 x 8
                uint8_t tile_number = sprites_[sprites_idx_].tile_number;
                uint16_t base =
                    PPUCTRL.SPRITE_PATTERN_ADDR * 0x1000 + tile_number * 16;
                uint16_t pattern_addr = 0;

                if (sprites_[sprites_idx_].flip_v) {
                  pattern_addr =
                      base + 7 - (scanline_ - sprites_[sprites_idx_].y);
                } else {
                  pattern_addr =
                      base + (scanline_ - sprites_[sprites_idx_].y);
                }
                sprites_[sprites_idx_].pattern_ls_shift =
                    ReadVRAM(pattern_addr);
                sprites_[sprites_idx_].pattern_ms_shift =
                    ReadVRAM(pattern_addr + 8);
              } else {
                // 8 x 16
              }

              if (sprites_[sprites_idx_].flip_h) {
                sprites_[sprites_idx_].pattern_ls_shift =
                    flip_h(sprites_[sprites_idx_].pattern_ls_shift);
                sprites_[sprites_idx_].pattern_ms_shift =
                    flip_h(sprites_[sprites_idx_].pattern_ms_shift);
              }
              break;
            }
            case 4: {
              // X coord
              sprites_[sprites_idx_].x = oam_[sprites_idx_ * 4 + (tmp - 1)];
              break;
            }
          }
        }
      }
    }
  }

  // Render
  if (scanline_ >= 0 && scanline_ <= 239 && cycles_ >= 1 && cycles_ <= 256) {
    uint8_t bg_palette_idx = 0;
    uint8_t sp_palette_idx = 0;
    Color final_color;

    // Background
    if (PPUMASK.BACKGROUND_RENDERING) {
      uint16_t mask = (0x8000 >> x);
      uint8_t plane0 = (bg_ls_shift & mask) > 0;
      uint8_t plane1 = (bg_ms_shift & mask) > 0;

      mask = (0x80 >> x);
      uint8_t als = (attr_ls_shift & mask) > 0;
      uint8_t ams = (attr_ms_shift & mask) > 0;

      bg_palette_idx = plane0 + plane1 * 2;

      if (cycles_ >= 1 && cycles_ <= 8) {
        if (PPUMASK.BACKGROUND == 0) {
          // Hide left 8 pixels of background
          bg_palette_idx = 0;
        }
      }

      uint8_t coloridx;
      if (bg_palette_idx == 0) {
        coloridx = ReadVRAM(0x3F00);
      } else {
        coloridx = ReadVRAM(0x3F00 + (ams * 2 + als) * 4 + bg_palette_idx);
      }

      final_color = kColors[coloridx];
    }

    // Sprite
    if (PPUMASK.SPRITE_RENDERING) {
      for (int i = 0; i < sprites_count_; ++i) {
        sprites_[i].x--;
        if (sprites_[i].x <= 0 && sprites_[i].x >= -7) {
          uint8_t plane0 = (sprites_[i].pattern_ls_shift & 0x80) > 0;
          uint8_t plane1 = (sprites_[i].pattern_ms_shift & 0x80) > 0;

          sp_palette_idx = plane0 + plane1 * 2;

          if (cycles_ >= 1 && cycles_ <= 8) {
            if (PPUMASK.SPRITES == 0) {
              // Hide left 8 pixels of sprite
              sp_palette_idx = 0;
            }
          }

          uint8_t palette = sprites_[i].palette;
          uint8_t coloridx = ReadVRAM(0x3F00 + (4 + palette) * 4 + sp_palette_idx);

          // Sprite 0 hit detect
          if (i == 0) {
            if (bg_palette_idx != 0 && sp_palette_idx != 0) {
              PPUSTATUS.SPRITE_HIT = 1;
            }
          }

          if (bg_palette_idx == 0) {
            if (sp_palette_idx != 0) {
              final_color = kColors[coloridx];
            } else {
              // Backdrop color
              final_color = kColors[ReadVRAM(0x3F00)];
            }
          } else {
            if (sp_palette_idx != 0 && !sprites_[i].priority) {
              final_color = kColors[coloridx];
            }
          }

          sprites_[i].pattern_ls_shift <<= 1;
          sprites_[i].pattern_ms_shift <<= 1;
        }
      }
    }
    if (PPUMASK.SPRITE_RENDERING || PPUMASK.BACKGROUND_RENDERING) {
      pixels_[scanline_ * 256 + (cycles_ - 1)] = final_color;
    }
  }

  // Pre scanline
  if (scanline_ == kScanLine) {
    // TODO(yangsiyu):
    if (cycles_ == 1) {
      PPUSTATUS.VBLANK = 0;
      if (PPUSTATUS.SPRITE_HIT) {
        PPUSTATUS.SPRITE_HIT = 0;
      }
    }
  }

  // VBLANK
  if (scanline_ == 241 && cycles_ == 1) {
    PPUSTATUS.VBLANK = 1;
    if (PPUCTRL.VBLANK_NMI) {
      cpu_.nmi_flipflop = true;
    }
  }

  cycles_++;
  if (cycles_ > kCycles) {
    scanline_++;

    // New scanline
    sprite_evaluation_state_ = kLessEight;
    oam_size_ = 0;
    n_overflow_ = false;
    n = 0;
    m = 0;
    sprites_idx_ = 0;

    if (scanline_ > kScanLine) {
      scanline_ = 0;
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

void PPU::TestRenderSprite() {
  const int kCellSize = 2;
  Color colors[] = {
    BLACK,
    WHITE,
    BLUE,
    RED,
  };

  for (unsigned int i = 0; i < OAM.size(); i += 4) {
    uint8_t sy = OAM[i];
    uint8_t sx = OAM[i + 3];

    uint8_t tile_id = OAM[i + 1];
    // uint8_t pattern_addr = (OAM[i + 1] & 0x1) * 0x1000;
    uint16_t pattern_addr = 0x1000;
    uint8_t flip_h = (OAM[i + 2] >> 6) & 0x1;
    uint8_t flip_v = (OAM[i + 2] >> 7) & 0x1;

    if (!flip_v) {
      // Top
      for (int j = tile_id * 16; j < tile_id * 16 + 8; ++j) {
        uint8_t plane0 = cartridge_.chr_rom[pattern_addr + j];
        uint8_t plane1 = cartridge_.chr_rom[pattern_addr + j + 8];

        if (!flip_h) {
          for (int k = 7; k >= 0; --k) {
            uint8_t plane0_bit = (plane0 >> k) & 0x1;
            uint8_t plane1_bit = (plane1 >> k) & 0x1;
            uint8_t color_idx = plane0_bit + plane1_bit * 2;

            DrawRectangle(sx * kCellSize + (7 - k) * kCellSize,
                          sy * kCellSize + (j - tile_id * 16) * kCellSize,
                          kCellSize, kCellSize, colors[color_idx]);
          }
        } else {
          for (int k = 0; k < 8; ++k) {
            uint8_t plane0_bit = (plane0 >> k) & 0x1;
            uint8_t plane1_bit = (plane1 >> k) & 0x1;
            uint8_t color_idx = plane0_bit + plane1_bit * 2;

            DrawRectangle(sx * kCellSize + k * kCellSize,
                          sy * kCellSize + (j - tile_id * 16) * kCellSize,
                          kCellSize, kCellSize, colors[color_idx]);
          }
        }
      }
    }
  }
}

void PPU::TestPalettes() {
  const int kCellSize = 30;
  int y = 0;
  int x = 0;
  for (int i = 0; i < palettes_.size(); i++) {
    DrawRectangle(100 + x, y, kCellSize, kCellSize, kColors[ReadVRAM(0x3F00 + i)]);
    x += kCellSize;
    if ((i + 1) % 4 == 0) {
      y += kCellSize + 10;
      x = 0;
    }
  }
}

uint8_t PPU::ReadVRAM(uint16_t addr) {
  if (addr > 0x3FFF) {
    nes_assert(false, std::format("Unsupported vram read: {:#x}", addr));
  }

  if (addr <= 0x1FFF) {
    // PATTERN TABLE
    return cartridge_.chr_rom[addr];
  } else if (addr >= 0x2000 && addr <= 0x23FF) {
    return vram_[addr - 0x2000];
  } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    return palettes_[(addr - 0x3F00) % 0x20];
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
    if (addr >= 0x2400 && addr <= 0x27FF) {
      return vram_[addr - 0x2000];
    } else if (addr >= 0x2800 && addr <= 0x2BFF) {
      return vram_[addr - 0x2800];
    } else if (addr >= 0x2C00 && addr <= 0x2FFF) {
      return vram_[addr - 0x2800];
    } else {
      nes_assert(false, std::format("Unsupported vram read: {:#x}", addr));
    }
  } else {
    nes_assert(false, std::format("Unsupported vram read: {:#x}", addr));
  }
}

void PPU::WriteVRAM(uint16_t addr, uint8_t v) {
  if (addr > 0x3FFF) {
    nes_assert(false, std::format("Unsupported vram write: {:#x}", addr));
  }

  if (addr >= 0x2000 && addr <= 0x23FF) {
    vram_[addr - 0x2000] = v;
  } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    uint16_t tmp_addr = (addr - 0x3F00) % 0x20;
    if (tmp_addr == 0x10) {
      palettes_[0] = v;
    }
    palettes_[(addr - 0x3F00) % 0x20] = v;
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 0) {
    if (addr >= 0x2400 && addr <= 0x27FF) {
      vram_[addr - 0x2400] = v;
    } else if (addr >= 0x2800 && addr <= 0x2BFF) {
      vram_[addr - 0x2400] = v;
    } else if (addr >= 0x2C00 && addr <= 0x2FFF) {
      vram_[addr - 0x2800] = v;
    }
  } else if (cartridge_.Flags6.NAMETABLE_ARRANGEMENT == 1) {
    if (addr >= 0x2400 && addr <= 0x27FF) {
      vram_[addr - 0x2000] = v;
    } else if (addr >= 0x2800 && addr <= 0x2BFF) {
      vram_[addr - 0x2800] = v;
    } else if (addr >= 0x2C00 && addr <= 0x2FFF) {
      vram_[addr - 0x2800] = v;
    }
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

uint8_t PPU::flip_h(uint8_t arg) {
  uint8_t result = 0;
  for (int i = 0; i < 8; ++i) {
    result <<= 1;
    uint8_t bit = (arg & (0x1 << i)) > 0;
    result |= bit;
  }
  return result;
}

}  // namespace nes
