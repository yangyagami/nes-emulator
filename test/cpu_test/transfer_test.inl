TEST(Transfer, TAX) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$32
    TAX

    LDA #$0
    TAX

    LDA #$ff
    TAX
  */
  uint8_t tmp[] = {
    0xa9, 0x32,
    0xaa,

    0xa9, 0x00,
    0xaa,

    0xa9, 0xff,
    0xaa
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
  EXPECT_EQ(cpu.A, 0x32);
  EXPECT_EQ(cpu.A, cpu.X);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.A, cpu.X);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.A, cpu.X);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(Transfer, TAY) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$32
    TAY

    LDA #$0
    TAY

    LDA #$ff
    TAY
  */
  uint8_t tmp[] = {
    0xa9, 0x32,
    0xa8,

    0xa9, 0x00,
    0xa8,

    0xa9, 0xff,
    0xa8
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
  EXPECT_EQ(cpu.A, 0x32);
  EXPECT_EQ(cpu.A, cpu.Y);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);
  EXPECT_EQ(cpu.A, cpu.Y);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.A, cpu.Y);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(Transfer, TSX) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$02
    PHA
    TSX

    LDA #$03
    PHA
    TSX
  */
  uint8_t tmp[] = {
    0xa9, 0x02,
    0x48,
    0xba,

    0xa9, 0x03,
    0x48,
    0xba,
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
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(memory[0x1ff], 0x02);
  EXPECT_EQ(cpu.X, 0xfe);
  EXPECT_EQ(cpu.SP, 0xfe);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x03);
  EXPECT_EQ(memory[0x1fe], 0x03);
  EXPECT_EQ(cpu.X, 0xfd);
  EXPECT_EQ(cpu.SP, 0xfd);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(Transfer, TXA) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDX #$32
    TXA

    LDX #$0
    TXA

    LDX #$ff
    TXA
  */
  uint8_t tmp[] = {
    0xa2, 0x32,
    0x8a,

    0xa2, 0x00,
    0x8a,

    0xa2, 0xff,
    0x8a
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
  EXPECT_EQ(cpu.X, 0x32);
  EXPECT_EQ(cpu.A, cpu.X);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x00);
  EXPECT_EQ(cpu.A, cpu.X);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xff);
  EXPECT_EQ(cpu.A, cpu.X);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(Transfer, TXS) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDX #$88
    LDA #$33
    TXS
  */
  uint8_t tmp[] = {
    0xa2, 0x88,
    0xa9, 0x33,
    0x9a,
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
  EXPECT_EQ(cpu.X, 0x88);
  EXPECT_EQ(cpu.A, 0x33);
  EXPECT_EQ(cpu.SP, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Transfer, TYA) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDY #$32
    TYA

    LDY #$88
    TYA

    LDA #$33
    TYA
  */
  uint8_t tmp[] = {
    0xa0, 0x32,
    0x98,

    0xa0, 0x88,
    0x98,

    0xa9, 0x33,
    0x98
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
  EXPECT_EQ(cpu.Y, 0x32);
  EXPECT_EQ(cpu.A, 0x32);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x88);
  EXPECT_EQ(cpu.A, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x88);
  EXPECT_EQ(cpu.A, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}