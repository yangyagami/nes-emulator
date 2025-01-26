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
