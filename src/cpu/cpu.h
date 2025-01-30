#ifndef NES_EMULATOR_CPU_CPU_H_
#define NES_EMULATOR_CPU_CPU_H_

#include <cstdint>
#include <string>
#include <map>
#include <functional>
#include <vector>

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
  union Status {
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

  bool nmi_flipflop;

  Cpu(Bus &bus);

  void Tick();
  void Reset();

  uint16_t GetAddress(AddressingMode addressing_mode);
  void UpdateZeroAndNegativeFlag(uint8_t v);
  void UpdateOverflowFlag(uint8_t a, uint8_t b, uint8_t result);
  void UpdateCarryFlag(int16_t result);
  void BranchIf(AddressingMode addressing, bool condition);
  void Compare(AddressingMode addressing, uint8_t reg);
  void Increment(uint8_t *target, int value);

  // Instructions
  void ADC(AddressingMode addressing);
  void AND(AddressingMode addressing);
  void ASL(AddressingMode addressing);

  void BCC(AddressingMode addressing);
  void BCS(AddressingMode addressing);
  void BEQ(AddressingMode addressing);
  void BIT(AddressingMode addressing);
  void BMI(AddressingMode addressing);
  void BNE(AddressingMode addressing);
  void BPL(AddressingMode addressing);
  void BVC(AddressingMode addressing);
  void BVS(AddressingMode addressing);
  void BRK(AddressingMode addressing);

  void CLC(AddressingMode addressing);
  void CLD(AddressingMode addressing);
  void CLI(AddressingMode addressing);
  void CLV(AddressingMode addressing);
  void CMP(AddressingMode addressing);
  void CPX(AddressingMode addressing);
  void CPY(AddressingMode addressing);

  void DEC(AddressingMode addressing);
  void DEX(AddressingMode addressing);
  void DEY(AddressingMode addressing);

  void EOR(AddressingMode addressing);

  void INC(AddressingMode addressing);
  void INX(AddressingMode addressing);
  void INY(AddressingMode addressing);

  void JMP(AddressingMode addressing);
  void JSR(AddressingMode addressing);

  void LDA(AddressingMode addressing);
  void LDX(AddressingMode addressing);
  void LDY(AddressingMode addressing);
  void LSR(AddressingMode addressing);

  void NOP(AddressingMode addressing);

  void ORA(AddressingMode addressing);

  void PHA(AddressingMode addressing);
  void PHP(AddressingMode addressing);
  void PLA(AddressingMode addressing);

  void ROL(AddressingMode addressing);
  void ROR(AddressingMode addressing);
  void RTI(AddressingMode addressing);
  void RTS(AddressingMode addressing);

  void SEC(AddressingMode addressing);
  void SED(AddressingMode addressing);
  void SEI(AddressingMode addressing);
  void STA(AddressingMode addressing);
  void STX(AddressingMode addressing);
  void STY(AddressingMode addressing);
  void SBC(AddressingMode addressing);

  void TAX(AddressingMode addressing);
  void TAY(AddressingMode addressing);
  void TSX(AddressingMode addressing);
  void TXA(AddressingMode addressing);
  void TXS(AddressingMode addressing);
  void TYA(AddressingMode addressing);

 private:
  // Some helper function
  bool IsCrossPage(uint16_t old_address, uint16_t new_address);
  uint16_t AbsoluteAdd(uint8_t reg);  // Absolute addressing with register.
  uint16_t ZeroPageAdd(uint8_t reg);  // ZeroPage addressing with register.
  void LoadToReg(uint8_t &reg, AddressingMode addressing);  // Used for LDA, LDX...
  void StoreToMem(uint8_t reg, AddressingMode addressing);  // Used for STA, STX...
  void Transfer(uint8_t from, uint8_t &to, bool p = true);  // Used for tax, tay..., p means whether update status register.
  void Push(uint8_t value);
  uint8_t Pop();

 private:
  Bus &bus_;
  bool jumped_;

#define NES_OPCODE(name, mode, code, bytes, cycles, func) \
  std::pair(code, \
            Opcode { name, mode, code, bytes, cycles, \
              std::bind(func, this, std::placeholders::_1)})

  const std::map<uint8_t, Opcode> kOpcodes = {
    NES_OPCODE("ADC", kImmediate,
               0x69, 2, 2, &Cpu::ADC),
    NES_OPCODE("ADC", kZeroPage,
               0x65, 2, 3, &Cpu::ADC),
    NES_OPCODE("ADC", kZeroPageX,
               0x75, 2, 4, &Cpu::ADC),
    NES_OPCODE("ADC", kAbsolute,
               0x6D, 3, 4, &Cpu::ADC),
    NES_OPCODE("ADC", kAbsoluteX,
               0x7D, 3, 4, &Cpu::ADC),
    NES_OPCODE("ADC", kAbsoluteY,
               0x79, 3, 4, &Cpu::ADC),
    NES_OPCODE("ADC", kIndexedIndirect,
               0x61, 2, 6, &Cpu::ADC),
    NES_OPCODE("ADC", kIndirectIndexed,
               0x71, 2, 5, &Cpu::ADC),

    NES_OPCODE("AND", kImmediate,
               0x29, 2, 2, &Cpu::AND),
    NES_OPCODE("AND", kZeroPage,
               0x25, 2, 3, &Cpu::AND),
    NES_OPCODE("AND", kZeroPageX,
               0x35, 2, 4, &Cpu::AND),
    NES_OPCODE("AND", kAbsolute,
               0x2D, 3, 4, &Cpu::AND),
    NES_OPCODE("AND", kAbsoluteX,
               0x3D, 3, 4, &Cpu::AND),
    NES_OPCODE("AND", kAbsoluteY,
               0x39, 3, 4, &Cpu::AND),
    NES_OPCODE("AND", kIndexedIndirect,
               0x21, 2, 6, &Cpu::AND),
    NES_OPCODE("AND", kIndirectIndexed,
               0x31, 2, 5, &Cpu::AND),

    NES_OPCODE("ASL", kImplicit,
               0x0A, 1, 2, &Cpu::ASL),
    NES_OPCODE("ASL", kZeroPage,
               0x06, 2, 5, &Cpu::ASL),
    NES_OPCODE("ASL", kZeroPageX,
               0x16, 2, 6, &Cpu::ASL),
    NES_OPCODE("ASL", kAbsolute,
               0x0E, 3, 6, &Cpu::ASL),
    NES_OPCODE("ASL", kAbsoluteX,
               0x1E, 3, 7, &Cpu::ASL),

    NES_OPCODE("BCC", kRelative,
               0x90, 2, 2, &Cpu::BCC),
    NES_OPCODE("BCS", kRelative,
               0xB0, 2, 2, &Cpu::BCS),
    NES_OPCODE("BEQ", kRelative,
               0xF0, 2, 2, &Cpu::BEQ),
    NES_OPCODE("BIT", kZeroPage,
               0x24, 2, 3, &Cpu::BIT),
    NES_OPCODE("BIT", kAbsolute,
               0x2C, 3, 4, &Cpu::BIT),
    NES_OPCODE("BMI", kRelative,
               0x30, 2, 2, &Cpu::BMI),
    NES_OPCODE("BNE", kRelative,
               0xD0, 2, 2, &Cpu::BNE),
    NES_OPCODE("BPL", kRelative,
               0x10, 2, 2, &Cpu::BPL),
    NES_OPCODE("BVC", kRelative,
               0x50, 2, 2, &Cpu::BVC),
    NES_OPCODE("BVS", kRelative,
               0x70, 2, 2, &Cpu::BVS),
    NES_OPCODE("BRK", kImplicit,
               0x00, 2, 7, &Cpu::BRK),

    NES_OPCODE("CLC", kImplicit,
               0x18, 1, 2, &Cpu::CLC),
    NES_OPCODE("CLD", kImplicit,
               0xD8, 1, 2, &Cpu::CLD),
    NES_OPCODE("CLI", kImplicit,
               0x58, 1, 2, &Cpu::CLI),
    NES_OPCODE("CLV", kImplicit,
               0xB8, 1, 2, &Cpu::CLV),
    NES_OPCODE("CMP", kImmediate,
               0xC9, 2, 2, &Cpu::CMP),
    NES_OPCODE("CMP", kZeroPage,
               0xC5, 2, 3, &Cpu::CMP),
    NES_OPCODE("CMP", kZeroPageX,
               0xD5, 2, 4, &Cpu::CMP),
    NES_OPCODE("CMP", kAbsolute,
               0xCD, 3, 4, &Cpu::CMP),
    NES_OPCODE("CMP", kAbsoluteX,
               0xDD, 3, 4, &Cpu::CMP),
    NES_OPCODE("CMP", kAbsoluteY,
               0xD9, 3, 4, &Cpu::CMP),
    NES_OPCODE("CMP", kIndexedIndirect,
               0xC1, 2, 6, &Cpu::CMP),
    NES_OPCODE("CMP", kIndirectIndexed,
               0xD1, 2, 5, &Cpu::CMP),
    NES_OPCODE("CPX", kImmediate,
               0xE0, 2, 2, &Cpu::CPX),
    NES_OPCODE("CPX", kZeroPage,
               0xE4, 2, 3, &Cpu::CPX),
    NES_OPCODE("CPX", kAbsolute,
               0xEC, 3, 4, &Cpu::CPX),
    NES_OPCODE("CPY", kImmediate,
               0xC0, 2, 2, &Cpu::CPY),
    NES_OPCODE("CPY", kZeroPage,
               0xC4, 2, 3, &Cpu::CPY),
    NES_OPCODE("CPY", kAbsolute,
               0xCC, 3, 4, &Cpu::CPY),

    NES_OPCODE("DEC", kZeroPage,
               0xC6, 2, 5, &Cpu::DEC),
    NES_OPCODE("DEC", kZeroPageX,
               0xD6, 2, 6, &Cpu::DEC),
    NES_OPCODE("DEC", kAbsolute,
               0xCE, 3, 6, &Cpu::DEC),
    NES_OPCODE("DEC", kAbsoluteX,
               0xDE, 3, 7, &Cpu::DEC),
    NES_OPCODE("DEX", kImplicit,
               0xCA, 1, 2, &Cpu::DEX),
    NES_OPCODE("DEY", kImplicit,
               0x88, 1, 2, &Cpu::DEY),

    NES_OPCODE("EOR", kImmediate,
               0x49, 2, 2, &Cpu::EOR),
    NES_OPCODE("EOR", kZeroPage,
               0x45, 2, 3, &Cpu::EOR),
    NES_OPCODE("EOR", kZeroPageX,
               0x55, 2, 4, &Cpu::EOR),
    NES_OPCODE("EOR", kAbsolute,
               0x4D, 3, 4, &Cpu::EOR),
    NES_OPCODE("EOR", kAbsoluteX,
               0x5D, 3, 4, &Cpu::EOR),
    NES_OPCODE("EOR", kAbsoluteY,
               0x59, 3, 4, &Cpu::EOR),
    NES_OPCODE("EOR", kIndexedIndirect,
               0x41, 2, 6, &Cpu::EOR),
    NES_OPCODE("EOR", kIndirectIndexed,
               0x51, 2, 5, &Cpu::EOR),

    NES_OPCODE("INC", kZeroPage,
               0xE6, 2, 5, &Cpu::INC),
    NES_OPCODE("INC", kZeroPageX,
               0xF6, 2, 6, &Cpu::INC),
    NES_OPCODE("INC", kAbsolute,
               0xEE, 3, 6, &Cpu::INC),
    NES_OPCODE("INC", kAbsoluteX,
               0xFE, 3, 7, &Cpu::INC),
    NES_OPCODE("INX", kImplicit,
               0xE8, 1, 2, &Cpu::INX),
    NES_OPCODE("INY", kImplicit,
               0xC8, 1, 2, &Cpu::INY),

    NES_OPCODE("JMP", kAbsolute,
               0x4C, 3, 3, &Cpu::JMP),
    NES_OPCODE("JMP", kIndirect,
               0x6C, 3, 5, &Cpu::JMP),
    NES_OPCODE("JSR", kAbsolute,
               0x20, 3, 6, &Cpu::JSR),

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
    NES_OPCODE("LSR", kImplicit,
               0x4A, 1, 2, &Cpu::LSR),
    NES_OPCODE("LSR", kZeroPage,
               0x46, 2, 5, &Cpu::LSR),
    NES_OPCODE("LSR", kZeroPageX,
               0x56, 2, 6, &Cpu::LSR),
    NES_OPCODE("LSR", kAbsolute,
               0x4E, 3, 6, &Cpu::LSR),
    NES_OPCODE("LSR", kAbsoluteX,
               0x5E, 3, 7, &Cpu::LSR),

    NES_OPCODE("NOP", kImplicit,
               0xEA, 1, 2, &Cpu::NOP),

    NES_OPCODE("ORA", kImmediate,
               0x09, 2, 2, &Cpu::ORA),
    NES_OPCODE("ORA", kZeroPage,
               0x05, 2, 3, &Cpu::ORA),
    NES_OPCODE("ORA", kZeroPageX,
               0x15, 2, 4, &Cpu::ORA),
    NES_OPCODE("ORA", kAbsolute,
               0x0D, 3, 4, &Cpu::ORA),
    NES_OPCODE("ORA", kAbsolute,
               0x0D, 3, 4, &Cpu::ORA),
    NES_OPCODE("ORA", kAbsoluteX,
               0x1D, 3, 4, &Cpu::ORA),
    NES_OPCODE("ORA", kAbsoluteY,
               0x19, 3, 4, &Cpu::ORA),
    NES_OPCODE("ORA", kIndexedIndirect,
               0x01, 2, 6, &Cpu::ORA),
    NES_OPCODE("ORA", kIndirectIndexed,
               0x11, 2, 5, &Cpu::ORA),

    NES_OPCODE("PHA", kImplicit,
               0x48, 1, 3, &Cpu::PHA),
    NES_OPCODE("PHP", kImplicit,
               0x08, 1, 3, &Cpu::PHP),
    NES_OPCODE("PLA", kImplicit,
               0x68, 1, 4, &Cpu::PLA),

    NES_OPCODE("ROL", kImplicit,
               0x2A, 1, 2, &Cpu::ROL),
    NES_OPCODE("ROL", kZeroPage,
               0x26, 2, 5, &Cpu::ROL),
    NES_OPCODE("ROL", kZeroPageX,
               0x36, 2, 6, &Cpu::ROL),
    NES_OPCODE("ROL", kAbsolute,
               0x2E, 3, 6, &Cpu::ROL),
    NES_OPCODE("ROL", kAbsoluteX,
               0x3E, 3, 7, &Cpu::ROL),
    NES_OPCODE("ROR", kImplicit,
               0x6A, 1, 2, &Cpu::ROR),
    NES_OPCODE("ROR", kZeroPage,
               0x66, 2, 5, &Cpu::ROR),
    NES_OPCODE("ROR", kZeroPageX,
               0x76, 2, 6, &Cpu::ROR),
    NES_OPCODE("ROR", kAbsolute,
               0x6E, 3, 6, &Cpu::ROR),
    NES_OPCODE("ROR", kAbsoluteX,
               0x7E, 3, 7, &Cpu::ROR),
    NES_OPCODE("RTI", kImplicit,
               0x40, 1, 6, &Cpu::RTI),
    NES_OPCODE("RTS", kImplicit,
               0x60, 1, 6, &Cpu::RTS),

    NES_OPCODE("SBC", kImmediate,
               0xE9, 2, 2, &Cpu::SBC),
    NES_OPCODE("SBC", kZeroPage,
               0xE5, 2, 3, &Cpu::SBC),
    NES_OPCODE("SBC", kZeroPageX,
               0xF5, 2, 4, &Cpu::SBC),
    NES_OPCODE("SBC", kAbsolute,
               0xED, 3, 4, &Cpu::SBC),
    NES_OPCODE("SBC", kAbsoluteX,
               0xFD, 3, 4, &Cpu::SBC),
    NES_OPCODE("SBC", kAbsoluteY,
               0xF9, 3, 4, &Cpu::SBC),
    NES_OPCODE("SBC", kIndexedIndirect,
               0xE1, 2, 6, &Cpu::SBC),
    NES_OPCODE("SBC", kIndirectIndexed,
               0xF1, 2, 5, &Cpu::SBC),
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
    NES_OPCODE("SEC", kImplicit,
               0x38, 1, 2, &Cpu::SEC),
    NES_OPCODE("SED", kImplicit,
               0xF8, 1, 2, &Cpu::SED),
    NES_OPCODE("SEI", kImplicit,
               0x78, 1, 2, &Cpu::SEI),

    NES_OPCODE("TAX", kImplicit,
               0xAA, 1, 2, &Cpu::TAX),
    NES_OPCODE("TAY", kImplicit,
               0xA8, 1, 2, &Cpu::TAY),
    NES_OPCODE("TSX", kImplicit,
               0xBA, 1, 2, &Cpu::TSX),
    NES_OPCODE("TXA", kImplicit,
               0x8A, 1, 2, &Cpu::TXA),
    NES_OPCODE("TXS", kImplicit,
               0x9A, 1, 2, &Cpu::TXS),
    NES_OPCODE("TYA", kImplicit,
               0x98, 1, 2, &Cpu::TYA),
  };

#undef NES_OPCODE
};

}  // namespace nes

#endif  // NES_EMULATOR_CPU_CPU_H_
