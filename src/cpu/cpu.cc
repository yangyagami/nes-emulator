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
  nes_assert(cycles == 0, std::format("Invalid cycles: {}", cycles));

  // Fetch opcode
  uint8_t opcode = bus_.CpuRead8Bit(PC);

  nes_assert(kOpcodes.find(opcode) != kOpcodes.end(),
             std::format("Invalid opcode: 0x{:02x}", opcode));

  Opcode opcode_obj = kOpcodes.find(opcode)->second;

  cycles = opcode_obj.cycles;

  opcode_obj.func(opcode_obj.addressing_mode);

  PC += opcode_obj.bytes;
}

void Cpu::Reset() {
  A = X = Y = 0;
  SP = 0XFF;
  P.raw = 0;
  P.B = 1;
  P.UNUSED = 1;

  cycles = 0;
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
    case kAbsoluteX: {
      result = AbsoluteAdd(X);
      break;
    }
    case kAbsoluteY: {
      result = AbsoluteAdd(Y);
      break;
    }
    case kZeroPage: {
      result = ZeroPageAdd(0);
      break;
    }
    case kZeroPageX: {
      result = ZeroPageAdd(X);
      break;
    }
    case kZeroPageY: {
      result = ZeroPageAdd(Y);
      break;
    }
    case kIndexedIndirect: {
      uint16_t addr = (PC + 1);
      uint8_t tmp = bus_.CpuRead8Bit(addr);

      tmp += X;

      result = bus_.CpuRead16Bit(tmp);
      break;
    }
    case kIndirectIndexed: {
      uint16_t addr = (PC + 1);
      uint8_t tmp = bus_.CpuRead8Bit(addr);
      uint16_t before = bus_.CpuRead16Bit(tmp);

      result = before + Y;

      if (IsCrossPage(before, result)) {
        cycles++;
      }
      break;
    }
    default:
      std::string msg = std::format("No such mode: {}",
                                    static_cast<int>(addressing_mode));
      nes_assert(false, msg);
      break;
  }

  return result;
}

void Cpu::UpdateZeroAndNegativeFlag(uint8_t v) {
  int8_t result = v;
  if (result == 0) {
    P.ZERO = 1;
  } else {
    P.ZERO = 0;
  }

  if (result < 0) {
    P.NEGATIVE = 1;
  } else {
    P.NEGATIVE = 0;
  }
}

// Instructions
void Cpu::LDA(AddressingMode addressing) {
  LoadToReg(A, addressing);
}

void Cpu::LDX(AddressingMode addressing) {
  LoadToReg(X, addressing);
}

void Cpu::LDY(AddressingMode addressing) {
  LoadToReg(Y, addressing);
}

void Cpu::STA(AddressingMode addressing) {
  StoreToMem(A, addressing);
}

void Cpu::STX(AddressingMode addressing) {
  StoreToMem(X, addressing);
}

void Cpu::STY(AddressingMode addressing) {
  StoreToMem(Y, addressing);
}

void Cpu::TAX(AddressingMode addressing) {
  (void) addressing;  // Because we was implied

  Transfer(A, X);
}

void Cpu::TAY(AddressingMode addressing) {
  (void) addressing;  // Because we was implied

  Transfer(A, Y);
}

bool Cpu::IsCrossPage(uint16_t old_address, uint16_t new_address) {
  const int kPageSize = 256;
  if (old_address / kPageSize == new_address / kPageSize) {
    return false;
  }
  return true;
}

uint16_t Cpu::AbsoluteAdd(uint8_t reg) {
  uint16_t result = 0;

  uint16_t before = bus_.CpuRead16Bit(PC + 1);
  result = before + reg;

  if (IsCrossPage(before, result)) {
    cycles++;
  }

  return result;
}

uint16_t Cpu::ZeroPageAdd(uint8_t reg) {
  uint16_t addr = (PC + 1);
  uint8_t result = bus_.CpuRead8Bit(addr);
  result += reg;

  return result;
}

void Cpu::LoadToReg(uint8_t &reg, AddressingMode addressing) {
  uint16_t new_address = GetAddress(addressing);
  reg = bus_.CpuRead8Bit(new_address);

  UpdateZeroAndNegativeFlag(reg);
}

void Cpu::StoreToMem(uint8_t reg, AddressingMode addressing) {
  uint16_t new_address = GetAddress(addressing);
  bus_.CpuWrite8Bit(new_address, reg);
}

void Cpu::Transfer(uint8_t from, uint8_t &to, bool p) {
  to = from;

  if (p) {
    UpdateZeroAndNegativeFlag(to);
  }
}

}  // namespace nes
