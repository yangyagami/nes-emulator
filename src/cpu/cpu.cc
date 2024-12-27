#include "cpu.h"

#include <format>
#include <iostream>
#include <cassert>
#include <string>

#include "bus/bus.h"
#include "utils/assert.h"

namespace nes {

Cpu::Cpu(Bus &bus) : bus_(bus) {}

void Cpu::Tick() {
  // Fetch opcode
  uint8_t opcode = bus_.CpuRead8Bit(PC);

  nes_assert(kOpcodes.find(opcode) != kOpcodes.end(),
             std::format("Invalid opcode: {:#02x}", opcode));

  Opcode opcode_obj = kOpcodes.find(opcode)->second;

  opcode_obj.func(opcode_obj.addressing_mode);

  PC += opcode_obj.bytes;
}

void Cpu::Reset() {
  A = X = Y = 0;
  SP = 0XFF;
  P.raw = 0;
  P.B = 1;
  P.UNUSED = 1;
}

uint16_t Cpu::GetAddress(AddressingMode addressing_mode) {
  uint16_t result = 0;

  switch (addressing_mode) {
    case kImmediate: {
      result = PC + 1;
      break;
    }
    case kAbsolute: {
      result = bus_.CpuRead16Bit(PC + 1);
      break;
    }
    default:
      assert(false);
      break;
  }

  return result;
}

void Cpu::UpdateZeroAndNegativeFlag(uint8_t v) {
  int8_t result = v;
  if (result == 0) {
    P.ZERO = 0;
  } else {
    P.ZERO = 1;
  }

  if (result < 0) {
    P.NEGATIVE = 1;
  } else {
    P.NEGATIVE = 0;
  }
}

// Instructions
void Cpu::LDA(AddressingMode addressing) {
  uint16_t new_address = GetAddress(addressing);
  A = bus_.CpuRead8Bit(new_address);

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::STA(AddressingMode addressing) {
  uint16_t new_address = GetAddress(addressing);
  bus_.CpuWrite8Bit(new_address, A);
}

}  // namespace nes
