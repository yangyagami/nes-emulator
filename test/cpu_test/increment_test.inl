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

TEST(DEC, AbsoluteX) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$01

    DEC $2221, X
    DEC $2221, X

    LDA #$20
    STA $2222
    DEC $2221, X

    LDA #$01
    STA $2221, X
    DEC $2221, X
   */
  uint8_t tmp[] = {
    0xa2, 0x01,
    0xde, 0x21, 0x22,
    0xde, 0x21, 0x22,

    0xa9, 0x20,
    0x8d, 0x22, 0x22,
    0xde, 0x21, 0x22,

    0xa9, 0x01,
    0x9d, 0x21, 0x22,
    0xde, 0x21, 0x22,
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

TEST(Decrement, Registers) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    DEX
    LDX #$01
    DEX

    DEY
    LDY #$01
    DEY
   */
  uint8_t tmp[] = {
    0xca,
    0xa2, 0x01,
    0xca,

    0x88,
    0xa0, 0x01,
    0x88,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
}

TEST(INC, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    test:
    INC $00
    LDA $00
    CMP #$FF
    BNE test
   */
  uint8_t tmp[] = {
    0xe6, 0x00,
    0xa5, 0x00,
    0xc9, 0xff,
    0xd0, 0xf8,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  while (cpu.PC != 0x0608) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(INC, ZeroPageX) {
  std::array<uint8_t, 0x10000> memory = { 0 };
  /*
    LDX #$32

    test:
    INC $00, X
    LDA $32
    CMP #$FF
    BNE test
   */
  uint8_t tmp[] = {
    0xa2, 0x32,

    0xf6, 0x00,
    0xa5, 0x32,
    0xc9, 0xff,
    0xd0, 0xf8,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  while (cpu.PC != 0x060A) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0xFF);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}
