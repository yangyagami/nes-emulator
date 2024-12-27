#ifndef NES_EMULATOR_CPU_CPU_H_
#define NES_EMULATOR_CPU_CPU_H_

#include <cstdint>
#include <string>
#include <map>
#include <functional>

namespace nes {

class Bus;

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

  Cpu(Bus &bus);

  void Tick();
  void Reset();

  uint16_t GetAddress(AddressingMode addressing_mode);
  void UpdateZeroAndNegativeFlag(uint8_t v);

  // Instructions
  void LDA(AddressingMode addressing);

  void STA(AddressingMode addressing);

 private:
  Bus &bus_;

#define NES_OPCODE(name, mode, code, bytes, cycles, func) \
  std::pair(code, \
            Opcode { name, mode, code, bytes, cycles, \
              std::bind(func, this, std::placeholders::_1)})

  const std::map<uint8_t, Opcode> kOpcodes = {
    NES_OPCODE("LDA", kImmediate,
               0xA9, 2, 2, &Cpu::LDA),

    NES_OPCODE("STA", kAbsolute,
               0x8D, 3, 4, &Cpu::STA),
  };

#undef NES_OPCODE
};

}  // namespace nes

#endif  // NES_EMULATOR_CPU_CPU_H_
