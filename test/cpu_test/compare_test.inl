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

TEST(CMP, ZeroPageX) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$05
    STA $22
    LDX #$2
    CMP $20, X

    LDA #$06
    CMP $20, X

    LDX #$FF
    STX $33
    LDX #$03
    CMP $30, X

    LDA #$FF
    LDX #$72
    STX $44
    LDX #$04
    CMP $40, X
   */
  uint8_t tmp[] = {
    0xa9, 0x05,
    0x85, 0x22,
    0xa2, 0x02,
    0xd5, 0x20,

    0xa9, 0x06,
    0xd5, 0x20,

    0xa2, 0xff,
    0x86, 0x33,
    0xa2, 0x03,
    0xd5, 0x30,

    0xa9, 0xff,
    0xa2, 0x72,
    0x86, 0x44,
    0xa2, 0x04,
    0xd5, 0x40
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
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}

TEST(CMP, Absolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDA #$FF
    STA $3300

    CMP $3300

    LDX #$08
    CMP $32F8, X

    LDY #$08
    CMP $32F8, Y
   */
  uint8_t tmp[] = {
    0xa9, 0xff,
    0x8d, 0x00, 0x33,

    0xcd, 0x00, 0x33,

    0xa2, 0x08,
    0xdd, 0xf8, 0x32,

    0xa0, 0x08,
    0xd9, 0xf8, 0x32
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
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(CMP, IndexedIndirect) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDY #$33
    STY $01
    LDY #$22
    STY $02

    LDX #$FF
    STX $2233

    LDA #$FF
    LDX #$01
    CMP ($00, X)
   */
  uint8_t tmp[] = {
    0xa0, 0x33,
    0x84, 0x01,
    0xa0, 0x22,
    0x84, 0x02,

    0xa2, 0xff,
    0x8e, 0x33, 0x22,

    0xa9, 0xff,
    0xa2, 0x01,
    0xc1, 0x00,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 6; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(CMP, IndirectIndexed) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDY #$00
    STY $01
    LDY #$22
    STY $02

    LDX #$FF
    STX $2233

    LDA #$FF
    LDY #$33
    CMP ($01), Y
   */
  uint8_t tmp[] = {
    0xa0, 0x00,
    0x84, 0x01,
    0xa0, 0x22,
    0x84, 0x02,

    0xa2, 0xff,
    0x8e, 0x33, 0x22,

    0xa9, 0xff,
    0xa0, 0x33,
    0xd1, 0x01,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  for (int i = 0; i < 6; ++i) {
    SafeTick(cpu);
  }

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110011);
}

TEST(CPX, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$80
    CPX #$80


    LDA #$70
    STA $03
    CPX $03

    LDA #$00
    STA $03
    CPX $0003
  */
  uint8_t tmp[] = {
    0xa2, 0x80,
    0xe0, 0x80,

    0xa9, 0x70,
    0x85, 0x03,
    0xe4, 0x03,

    0xa9, 0x00,
    0x85, 0x03,
    0xec, 0x03, 0x00,
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
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}

TEST(CPY, All) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDY #$80
    CPY #$80


    LDA #$70
    STA $03
    CPY $03

    LDA #$FF
    STA $03
    CPY $0003
  */
  uint8_t tmp[] = {
    0xa0, 0x80,
    0xc0, 0x80,

    0xa9, 0x70,
    0x85, 0x03,
    0xc4, 0x03,

    0xa9, 0xff,
    0x85, 0x03,
    0xcc, 0x03, 0x00,
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
  EXPECT_EQ(cpu.P.raw, 0b00110011);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b00110001);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}
