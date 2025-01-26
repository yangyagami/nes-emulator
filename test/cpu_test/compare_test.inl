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
