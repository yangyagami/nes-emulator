#include <iostream>
#include <cstdint>
#include <array>
#include <cassert>

#include "raylib.h"
#include "gtest/gtest.h"

#include "cpu/cpu.h"
#include "bus/bus.h"

void SafeTick(nes::Cpu &cpu) {
  cpu.Tick();
  while (--cpu.cycles > 0);
}

TEST(LDA, LDA_AbsoluteAddressing) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$02
    STA $0003
    LDA #$05
    LDA $0003
  */
  uint8_t tmp[] = {
    0xa9, 0x02,
    0x8d, 0x03, 0x00,
    0xa9, 0x05,
    0xad, 0x03, 0x00,
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

  EXPECT_EQ(cpu.A, 2);
  EXPECT_EQ(cpu.P.raw, 0x30);
}

TEST(LDA, LDA_Immediately) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$05
    LDA #$FF
    LDA #$00
   */
  uint8_t tmp[] = {
    0xa9 ,0x05,
    0xa9, 0xff,
    0xa9, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);

  EXPECT_EQ(cpu.A, 5);
  EXPECT_EQ(cpu.P.raw, 0x30);

  SafeTick(cpu);

  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0xb0);

  SafeTick(cpu);

  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
}

TEST(LDA, LDA_ZeroPage) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$32
    STA $0000
    LDA #$00
    LDA $00
    LDA #$32
   */
  uint8_t tmp[] = {
    0xa9, 0x32,
    0x8d, 0x00, 0x00,
    0xa9 ,0x00,
    0xa5 ,0x00,
    0xa9, 0x32,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x32);
  EXPECT_EQ(cpu.P.raw, 0x30);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x32);
  EXPECT_EQ(cpu.P.raw, 0x30);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x32);
  EXPECT_EQ(cpu.P.raw, 0x30);
}

TEST(LDA, LDA_ZeroPageX) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$05
    STA $0010
    LDA #$01
    LDA $10, X

    LDA #$05
    STA $0002
    LDX #$F
    LDA $F3, X
   */
  uint8_t tmp[] = {
    0xa9, 0x05,
    0x8d, 0x10, 0x00,
    0xa9, 0x01,
    0xb5, 0x10,

    0xa9, 0x05,
    0x8d, 0x02, 0x00,
    0xa2, 0x0f,
    0xb5, 0xf3
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x5);
  EXPECT_EQ(cpu.P.raw, 0x30);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x1);
  EXPECT_EQ(cpu.P.raw, 0x30);
  EXPECT_EQ(memory[0x10], 0x5);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x5);
  EXPECT_EQ(cpu.P.raw, 0x30);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x5);
  EXPECT_EQ(cpu.P.raw, 0x30);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x02], 0x5);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xF);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x5);
  EXPECT_EQ(cpu.P.raw, 0x30);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  // // range($0200, $05ff) used to draw pixels.
  // int sw = 32 * 20;
  // int sh = 32 * 20;
  // InitWindow(sw, sh, "easy test");
  // SetTargetFPS(60);

  // Color colors[] = {
  //   BLACK,
  //   BLACK,
  //   RED,
  //   GREEN,
  //   BLUE,
  //   PURPLE,
  //   ORANGE,
  //   YELLOW,
  //   GRAY,
  //   LIGHTGRAY,
  //   WHITE,
  // };

  // while (!WindowShouldClose()) {
  //   BeginDrawing();
  //   ClearBackground(BLACK);
  //   int x = 0;
  //   int y = 0;
  //   for (uint16_t i = 0x0200; i <= 0x05ff; ++i) {
  //     if (i != 0x0200 && i % 32 == 0) {
  //       x = 0;
  //       y++;
  //     }
  //     DrawRectangle((x++) * 20, y * 20, 20, 20,
  //                   colors[memory[i]]);
  //   }
  //   EndDrawing();
  // }
  return 0;
}
