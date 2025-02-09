#include <iostream>
#include <cstdint>
#include <array>
#include <cassert>

#include "raylib.h"
#include "gtest/gtest.h"

#include "cpu/cpu.h"
#include "bus.h"

void SafeTick(nes::Cpu &cpu) {
  cpu.Tick();
  while (--cpu.cycles > 0);
}

#include "adc_test.inl"
#include "load_test.inl"
#include "transfer_test.inl"
#include "branch_test.inl"
#include "compare_test.inl"
#include "increment_test.inl"

TEST(AND, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$02
    STX $1200
    STX $20

    LDX #$01
    LDY #$01

    LDA #$00
    AND #$02

    LDA #$02
    AND #$02

    AND $20
    AND $1f, X

    AND $1200
    AND $11ff, X
    AND $11ff, Y

    LDA #$12
    STA $02
    AND ($00, X)
    LDY #$00
    AND ($01), Y
   */
  uint8_t tmp[] = {
    0xa2, 0x02,
    0x8e, 0x00, 0x12,
    0x86, 0x20,

    0xa2, 0x01,
    0xa0, 0x01,

    0xa9, 0x00,
    0x29, 0x02,

    0xa9, 0x02,
    0x29, 0x02,

    0x25, 0x20,
    0x35, 0x1f,

    0x2d, 0x00, 0x12,
    0x3d, 0xff, 0x11,
    0x39, 0xff, 0x11,

    0xa9, 0x12,
    0x85, 0x02,
    0x21, 0x00,
    0xa0, 0x00,
    0x31, 0x01,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 5; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  for (int i = 0; i < 5; ++i) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(ASL, ALL) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$80
    ASL

    LDA #$01
    STA $33
    ASL $33

    LDX #$03
    ASL $30, X

    ASL $0033
    ASL $0030, X

    LDA #$80
    STA $33
    ASL $33
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x0a,

    0xa9, 0x01,
    0x85, 0x33,
    0x06, 0x33,

    0xa2, 0x03,
    0x16, 0x30,

    0x0e, 0x33, 0x00,
    0x1e, 0x30, 0x00,

    0xa9, 0x80,
    0x85, 0x33,
    0x06, 0x33,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x01);
  EXPECT_EQ(memory[0x33], 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x03);
  EXPECT_EQ(memory[0x33], 0x04);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x33], 0x08);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x33], 0x10);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(memory[0x33], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(BIT, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$80
    STX $20
    BIT $20

    LDX #$80
    STX $20
    BIT $20

    LDX #$70
    STX $20
    BIT $20
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x86, 0x20,
    0x24, 0x20,

    0xa2, 0x80,
    0x86, 0x20,
    0x24, 0x20,

    0xa2, 0x70,
    0x86, 0x20,
    0x24, 0x20,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(cpu.X, 0x80);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(cpu.X, 0x70);
  EXPECT_EQ(cpu.P.raw, 0b01110010);
}

TEST(BIT, Absolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$80
    STX $1220
    BIT $1220

    LDX #$80
    STX $1220
    BIT $1220

    LDX #$70
    STX $1220
    BIT $1220
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x8e, 0x20, 0x12,
    0x2c, 0x20, 0x12,

    0xa2, 0x80,
    0x8e, 0x20, 0x12,
    0x2c, 0x20, 0x12,

    0xa2, 0x70,
    0x8e, 0x20, 0x12,
    0x2c, 0x20, 0x12,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(cpu.X, 0x80);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  EXPECT_EQ(cpu.X, 0x70);
  EXPECT_EQ(cpu.P.raw, 0b01110010);
}

TEST(BRK, EasyTest) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    ;; Preset ($FFFE) = $3231
    BRK
  */
  uint8_t tmp[] = { 0x00 };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  memory[0xFFFE] = 0x31;
  memory[0xFFFF] = 0x32;

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x3231);
  EXPECT_EQ(memory[0x1FF], 0x02);
  EXPECT_EQ(memory[0x1FE], 0x06);
  EXPECT_EQ(memory[0x1FD], 0b00110000);
}

TEST(EOR, Immediate) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    EOR #$01
    EOR #$FF

    LDA #$00
    EOR #$FF

    LDA #$00
    EOR #$00
   */
  uint8_t tmp[] = {
    0x49, 0x01,
    0x49, 0xff,

    0xa9, 0x00,
    0x49, 0xff,

    0xa9, 0x00,
    0x49, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x01);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFE);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
}

TEST(EOR, kZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    EOR $22

    LDA #$FF
    STA $22

    LDA #$00
    EOR $22
   */
  uint8_t tmp[] = {
    0x45, 0x22,

    0xa9, 0xff,
    0x85, 0x22,

    0xa9, 0x00,
    0x45, 0x22,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(EOR, kZeroPageX) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$22
    EOR $22, X

    LDA #$FF
    STA $22, X

    LDA #$00
    EOR $22, X
   */
  uint8_t tmp[] = {
    0xa2, 0x22,
    0x55, 0x22,

    0xa9, 0xff,
    0x95, 0x22,

    0xa9, 0x00,
    0x55, 0x22,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(EOR, kAbsolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$00
    EOR $2136

    LDA #$FF
    STA $2136

    LDA #$00
    EOR $2136
   */
  uint8_t tmp[] = {
    0xa2, 0x00,
    0x4d, 0x36, 0x21,

    0xa9, 0xff,
    0x8d, 0x36, 0x21,

    0xa9, 0x00,
    0x4d, 0x36, 0x21,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(EOR, kAbsoluteX) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$00
    EOR $2136, X

    LDA #$FF
    STA $2136

    LDA #$00
    EOR $2136, X
   */
  uint8_t tmp[] = {
    0xa2, 0x00,
    0x5d, 0x36, 0x21,

    0xa9, 0xff,
    0x8d, 0x36, 0x21,

    0xa9, 0x00,
    0x5d, 0x36, 0x21,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(EOR, kAbsoluteY) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDY #$00
    EOR $2136, Y

    LDA #$FF
    STA $2136

    LDA #$00
    EOR $2136, Y
   */
  uint8_t tmp[] = {
    0xa0, 0x00,
    0x59, 0x36, 0x21,

    0xa9, 0xff,
    0x8d, 0x36, 0x21,

    0xa9, 0x00,
    0x59, 0x36, 0x21
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(EOR, kIndexedIndirect) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$36
    STA $00
    LDA #$21
    STA $01

    EOR ($00, X)

    LDA #$FF
    STA $2136

    LDA #$00
    EOR ($00, X)
   */
  uint8_t tmp[] = {
    0xa9, 0x36,
    0x85, 0x00,
    0xa9, 0x21,
    0x85, 0x01,

    0x41, 0x00,

    0xa9, 0xff,
    0x8d, 0x36, 0x21,

    0xa9, 0x00,
    0x41, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 4; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x36);
  EXPECT_EQ(memory[0x01], 0x21);
  EXPECT_EQ(cpu.A, 0x21);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(EOR, kIndirectIndexed) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$02
    STA $00
    LDA #$00
    STA $01

    LDY #$1

    EOR ($00), Y

    LDA #$FF
    STA $0003

    LDA #$00
    EOR ($00), Y
   */
  uint8_t tmp[] = {
    0xa9, 0x02,
    0x85, 0x00,
    0xa9, 0x00,
    0x85, 0x01,

    0xa0, 0x01,

    0x51, 0x00,

    0xa9, 0xff,
    0x8d, 0x03, 0x00,

    0xa9, 0x00,
    0x51, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 5; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(JSR, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    JSR init
    JSR loop
    JSR end

    init:
    LDX #$00
    RTS

    loop:
    INX
    CPX #$05
    BNE loop
    RTS

    end:
    BRK
   */
  uint8_t tmp[] = {
    0x20, 0x09, 0x06,
    0x20, 0x0c, 0x06,
    0x20, 0x12, 0x06,

    0xa2, 0x00,
    0x60,

    0xe8,
    0xe0, 0x05,
    0xd0, 0xfb,
    0x60,

    0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0609);

  // init
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0603);

  // loop
  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x060c);

  while (cpu.PC != 0x0612) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.PC, 0x0612);

}

TEST(LSR, Implicit) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDA #$01
    LSR

    LDA #$30
    LSR

    LDA #$FF
    LSR

    LDA #$8F
    LSR
   */
  uint8_t tmp[] = {
    0xa9, 0x01,
    0x4a,

    0xa9, 0x30,
    0x4a,

    0xa9, 0xff,
    0x4a,

    0xa9, 0x8f,
    0x4a,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x18);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x7F);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x47);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(LSR, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDA #$01
    STA $22
    LSR $22

    LDA #$30
    STA $22
    LSR $22

    LDA #$FF
    STA $22
    LSR $22

    LDA #$8F
    STA $22
    LSR $22
   */
  uint8_t tmp[] = {
    0xa9, 0x01,
    0x85, 0x22,
    0x46, 0x22,

    0xa9, 0x30,
    0x85, 0x22,
    0x46, 0x22,

    0xa9, 0xff,
    0x85, 0x22,
    0x46, 0x22,

    0xa9, 0x8f,
    0x85, 0x22,
    0x46, 0x22,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x22], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x22], 0x18);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x22], 0x7F);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x22], 0x47);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(LSR, ZeroPageX) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$22

    LDA #$01
    STA $22, X
    LSR $22, X

    LDA #$30
    STA $22, X
    LSR $22, X

    LDA #$FF
    STA $22, X
    LSR $22, X

    LDA #$8F
    STA $22, X
    LSR $22, X
   */
  uint8_t tmp[] = {
    0xa2, 0x22,

    0xa9, 0x01,
    0x95, 0x22,
    0x56, 0x22,

    0xa9, 0x30,
    0x95, 0x22,
    0x56, 0x22,

    0xa9, 0xff,
    0x95, 0x22,
    0x56, 0x22,

    0xa9, 0x8f,
    0x95, 0x22,
    0x56, 0x22,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x44], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x44], 0x18);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x44], 0x7F);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x44], 0x47);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(LSR, Absolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$22

    LDA #$01
    STA $2200
    LSR $2200

    LDA #$30
    STA $2200
    LSR $2200

    LDA #$FF
    STA $2200
    LSR $2200

    LDA #$8F
    STA $2200
    LSR $2200
   */
  uint8_t tmp[] = {
    0xa2, 0x22,

    0xa9, 0x01,
    0x8d, 0x00, 0x22,
    0x4e, 0x00, 0x22,

    0xa9, 0x30,
    0x8d, 0x00, 0x22,
    0x4e, 0x00, 0x22,

    0xa9, 0xff,
    0x8d, 0x00, 0x22,
    0x4e, 0x00, 0x22,

    0xa9, 0x8f,
    0x8d, 0x00, 0x22,
    0x4e, 0x00, 0x22,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2200], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2200], 0x18);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2200], 0x7F);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2200], 0x47);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(LSR, AbsoluteX) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$22

    LDA #$01
    STA $2200, X
    LSR $2200, X

    LDA #$30
    STA $2200, X
    LSR $2200, X

    LDA #$FF
    STA $2200, X
    LSR $2200, X

    LDA #$8F
    STA $2200, X
    LSR $2200, X
   */
  uint8_t tmp[] = {
    0xa2, 0x22,

    0xa9, 0x01,
    0x9d, 0x00, 0x22,
    0x5e, 0x00, 0x22,

    0xa9, 0x30,
    0x9d, 0x00, 0x22,
    0x5e, 0x00, 0x22,

    0xa9, 0xff,
    0x9d, 0x00, 0x22,
    0x5e, 0x00, 0x22,

    0xa9, 0x8f,
    0x9d, 0x00, 0x22,
    0x5e, 0x00, 0x22,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2222], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2222], 0x18);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2222], 0x7F);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2222], 0x47);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(ORA, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$02
    STX $1200
    STX $20

    LDX #$01
    LDY #$01

    LDA #$00
    ORA #$02

    LDA #$02
    ORA #$02

    ORA $20
    ORA $1f, X

    ORA $1200
    ORA $11ff, X
    ORA $11ff, Y

    LDA #$12
    STA $02
    ORA ($00, X)
    LDY #$00
    ORA ($01), Y

    ORA #$FF
   */
  uint8_t tmp[] = {
    0xa2, 0x02,
    0x8e, 0x00, 0x12,
    0x86, 0x20,

    0xa2, 0x01,
    0xa0, 0x01,

    0xa9, 0x00,
    0x09, 0x02,

    0xa9, 0x02,
    0x09, 0x02,

    0x05, 0x20,
    0x15, 0x1f,

    0x0d, 0x00, 0x12,
    0x1d, 0xff, 0x11,
    0x19, 0xff, 0x11,

    0xa9, 0x12,
    0x85, 0x02,
    0x01, 0x00,
    0xa0, 0x00,
    0x11, 0x01,

    0x09, 0xFF,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 5; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  for (int i = 0; i < 5; ++i) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(ROL, Implicit) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDA #$80
    ROL
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x2a,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(ROL, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDA #$80
    STA $00

    SEC
    ROL $00
    ROL $00, X
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x85, 0x00,

    0x38,
    0x26, 0x00,
    0x36, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x01);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x03);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(ROL, Absolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDA #$80
    STA $00

    SEC
    ROL $0000
    ROL $0000, X
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x85, 0x00,

    0x38,
    0x2e, 0x00, 0x00,
    0x3e, 0x00, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x01);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x03);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(ROR, kImplicit) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDA #$01
    ROR
   */
  uint8_t tmp[] = {
    0xa9, 0x01,
    0x6a
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(ROR, Remain) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    ADC #$80
    ADC #$80

    ROR $00
    ROR $00, X

    ROR $0000
    ROR $0000, X
   */
  uint8_t tmp[] = {
    0x69, 0x80,
    0x69, 0x80,

    0x66, 0x00,
    0x76, 0x00,

    0x6e, 0x00, 0x00,
    0x7e, 0x00, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0], 0x80);
  EXPECT_EQ(cpu.P.raw, 0b11110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0], 0x40);
  EXPECT_EQ(cpu.P.raw, 0b01110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0], 0x20);
  EXPECT_EQ(cpu.P.raw, 0b01110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0], 0x10);
  EXPECT_EQ(cpu.P.raw, 0b01110000);
}

TEST(RTI, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    RTI
   */
  uint8_t tmp[] = {
    0x40,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0);
  EXPECT_EQ(cpu.SP, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Stack, PHA) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$33
    PHA
    PHA
    PHA
   */
  uint8_t tmp[] = {
    0xa9, 0x33,
    0x48,
    0x48,
    0x48,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x33);
  EXPECT_EQ(memory[0x1ff], 0x33);
  EXPECT_EQ(memory[0x1fe], 0x33);
  EXPECT_EQ(memory[0x1fd], 0x33);
  EXPECT_EQ(cpu.SP, 0xfc);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Stack, PHP) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    PHP

    LDY #$80

    PHP
   */
  uint8_t tmp[] = {
    0x08,

    0xa0, 0x80,

    0x08,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.SP, 0xfe);
  EXPECT_EQ(memory[0x1ff], 0x30);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.SP, 0xfd);
  EXPECT_EQ(memory[0x1fe], 0xb0);
}

TEST(Stack, PLA) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$58
    PHA

    LDA #$33
    PLA

    LDA #$88
    PHA

    LDA #$01
    PLA
   */
  uint8_t tmp[] = {
    0xa9, 0x58,
    0x48,

    0xa9, 0x33,
    0x68,

    0xa9, 0x88,
    0x48,

    0xa9, 0x01,
    0x68,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x58);
  EXPECT_EQ(cpu.SP, 0xfe);
  EXPECT_EQ(memory[0x1ff], 0x58);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x33);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x58);
  EXPECT_EQ(cpu.SP, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x88);
  EXPECT_EQ(cpu.SP, 0xfe);
  EXPECT_EQ(memory[0x1ff], 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x1);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x88);
  EXPECT_EQ(cpu.SP, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(SBC, test) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    init:
    LDX #$F0
    LDY #$E0
    LDA #$11
    STA $00F2
    STA $00E2

    overflow:
    LDA #$80
    SBC #$7f
    ;; A=$00 P=0b01110011

    negative:
    CLC
    LDA #$11
    SBC $F2

    zero_carry:
    SEC
    LDA #$11
    SBC $02, X

    carry_clear:
    LDA #$05
    SBC $00F2

    only_carry:
    LDA #$22
    SBC $0002, X
    LDA #$22
    SBC $0002, Y
  */
  uint8_t tmp[] = {
    0xa2, 0xf0,
    0xa0, 0xe0,
    0xa9, 0x11,
    0x8d, 0xf2, 0x00,
    0x8d, 0xe2, 0x00,

    0xa9, 0x80,
    0xe9, 0x7f,

    0x18,
    0xa9, 0x11,
    0xe5, 0xf2,

    0x38,
    0xa9, 0x11,
    0xf5, 0x02,

    0xa9, 0x05,
    0xed, 0xf2, 0x00,

    0xa9, 0x22,
    0xfd, 0x02, 0x00,
    0xa9, 0x22,
    0xf9, 0x02, 0x00,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 5; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b01110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xF4);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x10);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x11);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(SBC, IndexedIndirect) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$01
    STA $20
    LDA #$02
    STA $21

    LDA #$12
    STA $0201

    LDA #$20
    LDX #$10
    SBC ($10, X)
  */
  uint8_t tmp[] = {
    0xa9, 0x01,
    0x85, 0x20,
    0xa9, 0x02,
    0x85, 0x21,

    0xa9, 0x12,
    0x8d, 0x01, 0x02,

    0xa9, 0x20,
    0xa2, 0x10,
    0xe1, 0x10,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 9; ++i) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0x0d);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(SBC, IndirectIndexed) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$01
    STA $20
    LDA #$02
    STA $21

    LDA #$12
    STA $0201

    LDA #$20
    SBC ($20), Y
  */
  uint8_t tmp[] = {
    0xa9, 0x01,
    0x85, 0x20,
    0xa9, 0x02,
    0x85, 0x21,

    0xa9, 0x12,
    0x8d, 0x01, 0x02,

    0xa9, 0x20,
    0xf1, 0x20,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 8; ++i) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0x0d);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(STA, STA) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$14
    STA $00       ;; ZeroPage
    LDX #$1
    STA $00, X    ;; ZeroPageX
    ;; Memory[$00] = $14, Memory[$01] = $14

    STA $3232     ;; Absolute
    ;; Memory[$3232] = $14

    STA $3232, X  ;; AbsoluteX
    ;; Memory[$3233] = $14

    LDY #$32
    STA $3232, Y  ;; AbsoluteY
    ;; Memory[$3264] = $14

    LDA #$20
    STA ($00, X)  ;; IndexedIndirect
    ;; Memory[$0014] = $20

    LDA #$35
    STA ($00), Y  ;; IndirectIndexed
    ;; Memory[$1446] = $35
  */
  uint8_t tmp[] = {
    0xa9, 0x14,
    0x85, 0x00,
    0xa2, 0x01,
    0x95, 0x00,
    0x8d, 0x32, 0x32,
    0x9d, 0x32, 0x32,
    0xa0, 0x32,
    0x99, 0x32, 0x32,
    0xa9, 0x20,
    0x81, 0x00,
    0xa9, 0x35,
    0x91, 0x00
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x14);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x14);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x01);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x01], 0x14);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x3232], 0x14);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x3233], 0x14);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x32);
  EXPECT_EQ(memory[0x3264], 0x14);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x20);
  EXPECT_EQ(memory[0x0014], 0x20);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x35);
  EXPECT_EQ(memory[0x1446], 0x35);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(STX, STX) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$aa
    STX $ff    ;; ZeroPage
    ;; Memory[$ff] = $aa

    LDY #$1
    STX $ff, Y ;; ZeroPageY
    ;; Memory[$00] = $aa

    STX $0100  ;; Absolute
    ;; Memory[$0100] = $aa
  */
  uint8_t tmp[] = {
    0xa2, 0xaa,
    0x86, 0xff,

    0xa0, 0x01,
    0x96, 0xff,

    0x8e, 0x00, 0x01
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xaa);
  EXPECT_EQ(memory[0xff], 0xaa);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x1);
  EXPECT_EQ(memory[0x00], 0xaa);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0100], 0xaa);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(STY, STY) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDY #$aa
    STY $ff    ;; ZeroPage
    ;; Memory[$ff] = $aa

    LDX #$1
    STY $ff, X ;; ZeroPageX
    ;; Memory[$00] = $aa

    STY $0100  ;; Absolute
    ;; Memory[$0100] = $aa
  */
  uint8_t tmp[] = {
    0xa0, 0xaa,
    0x84, 0xff,

    0xa2, 0x01,
    0x94, 0xff,

    0x8c, 0x00, 0x01,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0xaa);
  EXPECT_EQ(memory[0xff], 0xaa);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x1);
  EXPECT_EQ(memory[0x00], 0xaa);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0100], 0xaa);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(ClearAndSet, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    SEC
    CLC

    SED
    CLD

    SEI
    CLI

    ADC #$80
    ADC #$80
    CLV
   */
  uint8_t tmp[] = {
    0x38,
    0x18,

    0xf8,
    0xd8,

    0x78,
    0x58,

    0x69, 0x80,
    0x69, 0x80,
    0xb8
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.P.CARRY, 1);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.CARRY, 0);

  SafeTick(cpu);
  EXPECT_EQ(cpu.P.DECIMAL, 1);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.DECIMAL, 0);

  SafeTick(cpu);
  EXPECT_EQ(cpu.P.INTERRUPT_DISABLE, 1);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.INTERRUPT_DISABLE, 0);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.OVERFLOW, 0);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  return 0;
}
