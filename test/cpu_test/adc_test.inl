TEST(ADC, Status) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    ADC #$32
    ADC #$50
    ADC #$FF
    ;; A=$81, P=0b10110001

    ADC #$80
    ADC #$80
    ADC #$7f
    ;; A=$80, P=0b11110000
  */
  uint8_t tmp[] = {
    0x69, 0x32,
    0x69, 0x50,
    0x69, 0xff,

    0x69, 0x80,
    0x69, 0x80,
    0x69, 0x7f,
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
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x82);
  EXPECT_EQ(cpu.P.raw, 0b11110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x81);
  EXPECT_EQ(cpu.P.raw, 0b10110001);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b01110001);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x83);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110001);
}

TEST(ADC, Status2) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    ADC #$ff
    ADC #$ff
    LDA #$ff
    ADC #$ff
   */
  uint8_t tmp[] = {
    0x69, 0xff,
    0x69, 0xff,
    0xa9, 0xff,
    0x69, 0xff,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xfe);
  EXPECT_EQ(cpu.P.raw, 0b10110001);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110001);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}

TEST(ADC, ZeroPage) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$25
    STX $33
    ADC $33
    ADC #$70
    ADC #$FF
   */
  uint8_t tmp[] = {
    0xa2, 0x25,
    0x86, 0x33,
    0x65, 0x33,
    0x69, 0x70,
    0x69, 0xff,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x25);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x33], 0x25);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x25);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x95);
  EXPECT_EQ(cpu.P.raw, 0b11110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x94);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}

TEST(ADC, ZeroPageX) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDY #$88
    LDX #$12
    STY $12
    ADC $00, X
   */
  uint8_t tmp[] = {
    0xa0, 0x88,
    0xa2, 0x12,
    0x84, 0x12,
    0x75, 0x00,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x12], 0x88);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(ADC, Absolute) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    ;; Absolute
    LDA #$45
    STA $1200
    ADC $1200
    ;; A=#$8a P=0b11110000

    ;; AbsoluteX
    LDX #$22
    LDA #$01
    STA $1200, X
    ADC $1200, X
    ;; A=#$02 P=0b00110000

    ;; AbsoluteY
    LDY #$05
    LDA #$ff
    STA $1200, Y
    ADC $1200, Y
    ;; A=#$fe P=0b10110001
   */
  uint8_t tmp[] = {
    0xa9, 0x45,
    0x8d, 0x00, 0x12,
    0x6d, 0x00, 0x12,

    0xa2, 0x22,
    0xa9, 0x01,
    0x9d, 0x00, 0x12,
    0x7d, 0x00, 0x12,

    0xa0, 0x05,
    0xa9, 0xff,
    0x99, 0x00, 0x12,
    0x79, 0x00, 0x12,
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
  EXPECT_EQ(cpu.A, 0x8a);
  EXPECT_EQ(cpu.P.raw, 0b11110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0xfe);
  EXPECT_EQ(cpu.P.raw, 0b10110001);
}

TEST(ADC, Indexed) {
  std::array<uint8_t, 0x10000> memory = { 0 };

  /*
    LDX #$01
    LDA #$05
    STA $01
    LDA #$07
    STA $02
    LDY #$0a
    STY $0705
    ADC ($00,X)

    LDY #$01
    LDA #$03
    STA $01
    LDA #$07
    STA $02
    LDX #$0a
    STX $0704
    ADC ($01),Y
  */
  uint8_t tmp[] = {
    0xa2, 0x01,
    0xa9, 0x05,
    0x85, 0x01,
    0xa9, 0x07,
    0x85, 0x02,
    0xa0, 0x0a,
    0x8c, 0x05, 0x07,
    0x61, 0x00,

    0xa0, 0x01,
    0xa9, 0x03,
    0x85, 0x01,
    0xa9, 0x07,
    0x85, 0x02,
    0xa2, 0x0a,
    0x8e, 0x04, 0x07,
    0x71, 0x01,
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
  EXPECT_EQ(cpu.A, 0x11);
  EXPECT_EQ(cpu.X, 0x01);
  EXPECT_EQ(cpu.Y, 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  for (int i = 0; i < 8; ++i) {
    SafeTick(cpu);
  }
  EXPECT_EQ(cpu.A, 0x11);
  EXPECT_EQ(cpu.X, 0x0a);
  EXPECT_EQ(cpu.Y, 0x01);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}
