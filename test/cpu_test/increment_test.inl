TEST(DEC, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    DEC $00
    DEC $00

    LDA #$20
    STA $00
    DEC $00

    LDA #$01
    STA $00
    DEC $00
   */
  uint8_t tmp[] = {
    0xc6, 0x00,
    0xc6, 0x00,

    0xa9, 0x20,
    0x85, 0x00,
    0xc6, 0x00,

    0xa9, 0x01,
    0x85, 0x00,
    0xc6, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0xFE);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x1F);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x00], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
}

TEST(DEC, ZeroPageX) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$01

    DEC $00, X
    DEC $00, X

    LDA #$20
    STA $01
    DEC $00, X

    LDA #$01
    STA $01
    DEC $00, X
   */
  uint8_t tmp[] = {
    0xa2, 0x01,

    0xd6, 0x00,
    0xd6, 0x00,

    0xa9, 0x20,
    0x85, 0x01,
    0xd6, 0x00,

    0xa9, 0x01,
    0x85, 0x01,
    0xd6, 0x00,
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
  EXPECT_EQ(memory[0x01], 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x01], 0xFE);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x01], 0x1f);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x01], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
}

TEST(DEC, Absolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$01

    DEC $2222
    DEC $2222

    LDA #$20
    STA $2222
    DEC $2222

    LDA #$01
    STA $2222
    DEC $2222
   */
  uint8_t tmp[] = {
    0xa2, 0x01,
    0xce, 0x22, 0x22,
    0xce, 0x22, 0x22,

    0xa9, 0x20,
    0x8d, 0x22, 0x22,
    0xce, 0x22, 0x22,

    0xa9, 0x01,
    0x8d, 0x22, 0x22,
    0xce, 0x22, 0x22,
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
  EXPECT_EQ(memory[0x2222], 0xFE);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2222], 0x1f);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(memory[0x2222], 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
}
