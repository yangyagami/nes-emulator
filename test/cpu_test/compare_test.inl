TEST(CMP, Immediate) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$05
    CMP #$05

    LDA #$06
    CMP #$05

    CMP #$FF

    LDA #$FF
    CMP #$70
   */
  uint8_t tmp[] = {
    0xa9, 0x05,
    0xc9, 0x05,

    0xa9, 0x06,
    0xc9, 0x05,

    0xc9, 0xff,

    0xa9, 0xff,
    0xc9, 0x70,
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
  EXPECT_EQ(cpu.A, 0x05);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x06);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}

TEST(CMP, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$05
    STA $22
    CMP $22

    LDA #$06
    CMP $22

    LDX #$FF
    STX $33
    CMP $33

    LDA #$FF
    LDX #$72
    STX $44
    CMP $44
   */
  uint8_t tmp[] = {
    0xa9, 0x05,
    0x85, 0x22,
    0xc5, 0x22,

    0xa9, 0x06,
    0xc5, 0x22,

    0xa2, 0xff,
    0x86, 0x33,
    0xc5, 0x33,

    0xa9, 0xff,
    0xa2, 0x72,
    0x86, 0x44,
    0xc5, 0x44,
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
  EXPECT_EQ(cpu.A, 0x05);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x06);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.X, 0x72);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}
