TEST(BCC, EasyTest) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$01
    test:
    ASL
    BCC test
   */

  uint8_t tmp[] = {
    0xa9, 0x01,
    0x0a,
    0x90, 0xfd
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

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x02);

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0602);

  // Jump to test
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x04);
}

TEST(BCC, EasyTest2) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    LDA #$80
    ADC #$80
    BCC test
    LDA #$05
   */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x69, 0x80,
    0x90, 0xfa,
    0xa9, 0x05,
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
  EXPECT_EQ(cpu.A, 0x05);
}

TEST(BCC, cycles) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    LDA #$80
    ADC #$80
    BCC test
    LDA #$05
  */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x69, 0x80,
    0x90, 0xfa,
    0xa9, 0x05,
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
  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 2);
}

TEST(BCC, cycles2) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    LDA #$01
    test:
    ASL
    BCC test
   */
  uint8_t tmp[] = {
    0xa9, 0x01,
    0x0a,
    0x90, 0xfd
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
  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 3);
  while (--cpu.cycles > 0);

  // PageCrossed test
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x05fd + i] = tmp[i];
  }
  cpu.Reset();
  cpu.PC = 0x05fd;

  SafeTick(cpu);
  SafeTick(cpu);
  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 4);
}

TEST(BCS, EasyTest) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    ADC #$80
    ADC #$80
    BCS test
   */
  uint8_t tmp[] = {
    0x69, 0x80,
    0x69, 0x80,
    0xb0, 0xfa,
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
  EXPECT_EQ(cpu.PC, 0x0600);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x81);
  EXPECT_EQ(cpu.P.raw, 0b10110000);
}

TEST(BCS, EasyTest2) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    ADC #$40
    ADC #$80
    BCS test

    ADC #$80
    BCS test
   */
  uint8_t tmp[] = {
    0x69, 0x40,
    0x69, 0x80,
    0xb0, 0xfa,

    0x69, 0x80,
    0xb0, 0xf6,
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
  cpu.Tick();
  EXPECT_EQ(cpu.PC, 0x0606);
  EXPECT_EQ(cpu.cycles, 2);
  while (--cpu.cycles > 0);

  SafeTick(cpu);
  cpu.Tick();
  EXPECT_EQ(cpu.PC, 0x0600);
  EXPECT_EQ(cpu.cycles, 3);

  // PageCrossed test
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x05fb + i] = tmp[i];
  }
  cpu.Reset();
  cpu.PC = 0x05fb;

  SafeTick(cpu);
  SafeTick(cpu);
  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 2);
  while (--cpu.cycles > 0);

  SafeTick(cpu);
  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 4);
}

TEST(BEQ, EasyTest) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    LDA #$35
    BEQ test
    LDA #$22
    LDA #$00
    BEQ test
  */

  uint8_t tmp[] = {
    0xa9, 0x35,
    0xf0, 0xfc,
    0xa9, 0x22,
    0xa9, 0x00,
    0xf0, 0xf6,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x35);

  cpu.Tick();
  EXPECT_EQ(cpu.PC, 0x0604);
  EXPECT_EQ(cpu.cycles, 2);
  while (--cpu.cycles > 0);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x22);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);

  cpu.Tick();
  EXPECT_EQ(cpu.PC, 0x0600);
  EXPECT_EQ(cpu.cycles, 3);
  while (--cpu.cycles > 0);

  cpu.Reset();
  cpu.PC = 0x05ff;
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[cpu.PC + i] = tmp[i];
  }

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x35);

  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 2);
  while (--cpu.cycles > 0);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x22);
  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x00);

  cpu.Tick();
  EXPECT_EQ(cpu.cycles, 4);
  while (--cpu.cycles > 0);
}

// Below branch, just check if jumped.
TEST(BMI, EasyTest) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    LDA #$80
    BMI test
  */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x30, 0xfc,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);
  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0600);
}

TEST(BNE, EasyTest) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    BNE test
  */
  uint8_t tmp[] = {
    0xd0, 0xfe,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0600);
}

TEST(BPL, EasyTest) {
  std::array<uint8_t, 0xFFFF> memory = { 0 };

  /*
    test:
    LDA #$80
    BPL test
    LDA #$01
    BPL test
  */
  uint8_t tmp[] = {
    0xa9, 0x80,
    0x10, 0xfc,
    0xa9, 0x01,
    0x10, 0xf8,
  };
  for (uint16_t i = 0; i < sizeof(tmp) / sizeof(tmp[0]); ++i) {
    memory[0x0600 + i] = tmp[i];
  }

  nes::Bus bus(memory);

  nes::Cpu cpu(bus);
  cpu.Reset();
  cpu.PC = 0x0600;

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x80);

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0604);

  SafeTick(cpu);
  EXPECT_EQ(cpu.A, 0x01);

  SafeTick(cpu);
  EXPECT_EQ(cpu.PC, 0x0600);
}
