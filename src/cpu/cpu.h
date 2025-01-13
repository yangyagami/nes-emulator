#ifndef NES_EMULATOR_CPU_CPU_H_
#define NES_EMULATOR_CPU_CPU_H_

#include <cstdint>
#include <string>
#include <map>
#include <functional>

namespace nes {

class Bus;

/*
  Reference:
    Overview: https://www.nesdev.org/wiki/CPU
    Opcodes: https://www.nesdev.org/obelisk-6502-guide/reference.html
    Addressing: https://skilldrick.github.io/easy6502/#addressing
*/
struct Cpu {
  enum AddressingMode {
    kAbsolute = 0,
    kZeroPage,
    kZeroPageX,
    kZeroPageY,
    kAbsoluteX,
    kAbsoluteY,
    kImmediate,
    kRelative,
    kImplicit,
    kIndirect,
    kIndexedIndirect,
    kIndirectIndexed
  };

  struct Opcode {
    std::string name;
    AddressingMode addressing_mode;
    uint8_t opcode;
    uint8_t bytes;
    int cycles;
    std::function<void(AddressingMode)> func;
  };

  uint8_t A;
  uint8_t X;
  uint8_t Y;
  uint16_t PC;
  uint8_t SP;
  union {
    struct {
      uint8_t CARRY : 1;
      uint8_t ZERO : 1;
      uint8_t INTERRUPT_DISABLE : 1;
      uint8_t DECIMAL : 1;
      uint8_t B : 1;
      uint8_t UNUSED : 1;
      uint8_t OVERFLOW : 1;
      uint8_t NEGATIVE : 1;
    };
    uint8_t raw;
  } P;

  uint8_t cycles;

  Cpu(Bus &bus);

  void Tick();
  void Reset();

  uint16_t GetAddress(AddressingMode addressing_mode);
  void UpdateZeroAndNegativeFlag(uint8_t v);

  // Instructions
  void LDA(AddressingMode addressing);
  void LDX(AddressingMode addressing);
  void LDY(AddressingMode addressing);

  void PHA(AddressingMode addressing);

  void STA(AddressingMode addressing);
  void STX(AddressingMode addressing);
  void STY(AddressingMode addressing);

  void TAX(AddressingMode addressing);
  void TAY(AddressingMode addressing);
  void TSX(AddressingMode addressing);
  void TXA(AddressingMode addressing);

 private:
  // Some helper function
  bool IsCrossPage(uint16_t old_address, uint16_t new_address);
  uint16_t AbsoluteAdd(uint8_t reg);  // Absolute addressing with register.
  uint16_t ZeroPageAdd(uint8_t reg);  // ZeroPage addressing with register.
  void LoadToReg(uint8_t &reg, AddressingMode addressing);  // Used for LDA, LDX...
  void StoreToMem(uint8_t reg, AddressingMode addressing);  // Used for STA, STX...
  void Transfer(uint8_t from, uint8_t &to, bool p = true);  // Used for tax, tay..., p means whether update status register.

 private:
  Bus &bus_;

#define NES_OPCODE(name, mode, code, bytes, cycles, func) \
  std::pair(code, \
            Opcode { name, mode, code, bytes, cycles, \
              std::bind(func, this, std::placeholders::_1)})

  const std::map<uint8_t, Opcode> kOpcodes = {
    NES_OPCODE("LDA", kImmediate,
               0xA9, 2, 2, &Cpu::LDA),
    NES_OPCODE("LDA", kZeroPage,
               0xA5, 2, 3, &Cpu::LDA),
    NES_OPCODE("LDA", kZeroPageX,
               0xB5, 2, 4, &Cpu::LDA),
    NES_OPCODE("LDA", kAbsolute,
               0xAD, 3, 4, &Cpu::LDA),
    NES_OPCODE("LDA", kAbsoluteX,
               0xBD, 3, 4, &Cpu::LDA),
    NES_OPCODE("LDA", kAbsoluteY,
               0xB9, 3, 4, &Cpu::LDA),
    NES_OPCODE("LDA", kIndexedIndirect,
               0xA1, 2, 6, &Cpu::LDA),
    NES_OPCODE("LDA", kIndirectIndexed,
               0xB1, 2, 5, &Cpu::LDA),

    NES_OPCODE("LDX", kImmediate,
               0xA2, 2, 2, &Cpu::LDX),
    NES_OPCODE("LDX", kZeroPage,
               0xA6, 2, 3, &Cpu::LDX),
    NES_OPCODE("LDX", kZeroPageY,
               0xB6, 2, 4, &Cpu::LDX),
    NES_OPCODE("LDX", kAbsolute,
               0xAE, 3, 4, &Cpu::LDX),
    NES_OPCODE("LDX", kAbsoluteY,
               0xBE, 3, 4, &Cpu::LDX),

    NES_OPCODE("LDY", kImmediate,
               0xA0, 2, 2, &Cpu::LDY),
    NES_OPCODE("LDY", kZeroPage,
               0xA4, 2, 3, &Cpu::LDY),
    NES_OPCODE("LDY", kZeroPageX,
               0xB4, 2, 4, &Cpu::LDY),
    NES_OPCODE("LDY", kAbsolute,
               0xAC, 3, 4, &Cpu::LDY),
    NES_OPCODE("LDY", kAbsoluteX,
               0xBC, 3, 4, &Cpu::LDY),

    NES_OPCODE("PHA", kImplicit,
               0x48, 1, 3, &Cpu::PHA),

    NES_OPCODE("STA", kZeroPage,
               0x85, 2, 3, &Cpu::STA),
    NES_OPCODE("STA", kZeroPageX,
               0x95, 2, 4, &Cpu::STA),
    NES_OPCODE("STA", kAbsolute,
               0x8D, 3, 4, &Cpu::STA),
    NES_OPCODE("STA", kAbsoluteX,
               0x9D, 3, 5, &Cpu::STA),
    NES_OPCODE("STA", kAbsoluteY,
               0x99, 3, 5, &Cpu::STA),
    NES_OPCODE("STA", kIndexedIndirect,
               0x81, 2, 6, &Cpu::STA),
    NES_OPCODE("STA", kIndirectIndexed,
               0x91, 2, 6, &Cpu::STA),

    NES_OPCODE("STX", kZeroPage,
               0x86, 2, 3, &Cpu::STX),
    NES_OPCODE("STX", kZeroPageY,
               0x96, 2, 4, &Cpu::STX),
    NES_OPCODE("STX", kAbsolute,
               0x8E, 3, 4, &Cpu::STX),

    NES_OPCODE("STY", kZeroPage,
               0x84, 2, 3, &Cpu::STY),
    NES_OPCODE("STY", kZeroPageX,
               0x94, 2, 4, &Cpu::STY),
    NES_OPCODE("STY", kAbsolute,
               0x8C, 3, 4, &Cpu::STY),


    NES_OPCODE("TAX", kImplicit,
               0xAA, 1, 2, &Cpu::TAX),
    NES_OPCODE("TAY", kImplicit,
               0xA8, 1, 2, &Cpu::TAY),
    NES_OPCODE("TSX", kImplicit,
               0xBA, 1, 2, &Cpu::TSX),
    NES_OPCODE("TXA", kImplicit,
               0x8A, 1, 2, &Cpu::TXA),
  };

#undef NES_OPCODE
};

}  // namespace nes

#endif  // NES_EMULATOR_CPU_CPU_H_
