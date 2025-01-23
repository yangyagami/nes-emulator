TEST(Load, LDA_Immediately) {
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

TEST(Load, LDA_ZeroPage) {
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

TEST(Load, LDA_ZeroPageX) {
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

TEST(Load, LDA_AbsoluteAddressing) {
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

TEST(Load, LDA_AbsoluteX) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA $0000, X
    LDX #$ff
    STX $00ff
    LDA $00ff, X
    LDA $0000, X
  */
  uint8_t tmp[] = {
    0xbd, 0x00, 0x00,
    0xa2, 0xff,
    0x8e, 0xff, 0x00,
    0xbd, 0xff, 0x00,
    0xbd, 0x00, 0x00,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0xff], 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 5);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
  while(--cpu.cycles > 0);

  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 4);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(Load, LDA_AbsoluteY) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA $0000, Y
    LDY #$ff
    STY $00ff
    LDA $00ff, Y
    LDA $0000, Y
  */
  uint8_t tmp[] = {
    0xb9, 0x00, 0x00,
    0xa0, 0xff,
    0x8C, 0xff, 0x00,
    0xb9, 0xff, 0x00,
    0xb9, 0x00, 0x00,
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0xff], 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 5);
  EXPECT_EQ(cpu.P.raw, 0b00110010);
  while(--cpu.cycles > 0);

  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 4);
  EXPECT_EQ(cpu.A, 0xff);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(Load, LDA_IndirectX) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDX #$01
    LDA #$05
    STA $01
    LDA #$07
    STA $02
    LDY #$0a
    STY $0705
    LDA ($00,X)
  */
  uint8_t tmp[] = {
    0xa2, 0x01,
    0xa9, 0x05,
    0x85, 0x01,
    0xa9, 0x07,
    0x85, 0x02,
    0xa0, 0x0a,
    0x8c, 0x05, 0x07,
    0xa1, 0x00
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x01);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x05);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x01], 0x05);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x07);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x02], 0x07);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0705], 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Load, LDA_IndirectY) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDY #$01
    LDA #$03
    STA $01
    LDA #$07
    STA $02
    LDX #$0a
    STX $0704
    LDA ($01),Y
  */
  uint8_t tmp[] = {
    0xa0, 0x01,
    0xa9, 0x03,
    0x85, 0x01,
    0xa9, 0x07,
    0x85, 0x02,
    0xa2, 0x0a,
    0x8e, 0x04, 0x07,
    0xb1, 0x01
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x1);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x3);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x01], 0x3);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x7);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x02], 0x7);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0704], 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x0a);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Load, LDX) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDX #$fa
    LDA #$33
    STA $32ff
    LDX $32ff
    LDX #$25
    STX $0040
    LDX $40
  */
  uint8_t tmp[] = {
    0xa2, 0xfa,
    0xa9, 0x33,
    0x8d, 0xff, 0x32,
    0xae, 0xff, 0x32,
    0xa2, 0x25,
    0x8e, 0x40, 0x00,
    0xa6, 0x40
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0xfa);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x33);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x32ff], 0x33);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x33);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x25);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0040], 0x25);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x25);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Load, LDX_ZeroPageY) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$12
    STA $008f
    LDY #$8f
    LDX $00, Y
  */
  uint8_t tmp[] = {
    0xa9, 0x12,
    0x8d, 0x8f, 0x00,
    0xa0, 0x8f,
    0xb6, 0x00
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x008f], 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x8f);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
}

TEST(Load, LDX_AbsoluteY) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$12
    STA $008f
    LDY #$00
    LDX $008f, Y
    LDY #$22
    STY $00b1
    LDX $008f, Y

    LDY #$75
    LDX $008f, Y
  */
  uint8_t tmp[] = {
    0xa9, 0x12,
    0x8d, 0x8f, 0x00,
    0xa0, 0x00,
    0xbe, 0x8f, 0x00,
    0xa0, 0x22,
    0x8c, 0xb1, 0x00,
    0xbe, 0x8f, 0x00,

    0xa0, 0x75,
    0xbe, 0x8f, 0x00
  };

  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x008f], 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x00);
  EXPECT_EQ(cpu.P.raw, 0b00110010);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x12);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x22);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x00b1], 0x22);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x22);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x75);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  memory[0x0104] = 0x77;

  cpu.Tick();
  EXPECT_EQ(cpu.X, 0x77);
  EXPECT_EQ(cpu.P.raw, 0b00110000);
  EXPECT_EQ(cpu.cycles, 5);
}

TEST(Load, LDY) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDY #$88     ;; immediate
    STY $0000
    STY $0088
    LDY $00      ;; ZeroPage
    LDX $88
    LDY $00, X   ;; ZeroPageX
    LDY $0088    ;; Absolute
    LDA #$89
    STA $0089
    LDX #$01
    LDY $0088, X ;; AbsoluteX
  */
  uint8_t tmp[] = {
    0xa0, 0x88,
    0x8c, 0x00, 0x00,
    0x8c, 0x88, 0x00,
    0xa4, 0x00,
    0xa6, 0x88,
    0xb4, 0x00,
    0xac, 0x88, 0x00,
    0xa9, 0x89,
    0x8d, 0x89, 0x00,
    0xa2, 0x01,
    0xbc, 0x88, 0x00
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
  EXPECT_EQ(memory[0x0000], 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x0088], 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x88);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x89);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(memory[0x89], 0x89);
  EXPECT_EQ(cpu.P.raw, 0b10110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.X, 0x1);
  EXPECT_EQ(cpu.P.raw, 0b00110000);

  SafeTick(cpu);
  EXPECT_EQ(cpu.Y, 0x89);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}
