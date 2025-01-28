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

  jumped_ = false;

  // Fetch opcode
  uint8_t opcode = bus_.CpuRead8Bit(PC);

  nes_assert(kOpcodes.find(opcode) != kOpcodes.end(),
             std::format("Invalid opcode: 0x{:02x}", opcode));

  Opcode opcode_obj = kOpcodes.find(opcode)->second;

  cycles = opcode_obj.cycles;

  opcode_obj.func(opcode_obj.addressing_mode);

  if (!jumped_) {
    PC += opcode_obj.bytes;
  }
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
    case kRelative: {
      int8_t tmp = bus_.CpuRead8Bit(PC + 1);
      result = PC + tmp;
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
    case kIndirect: {
      uint16_t addr = bus_.CpuRead16Bit(PC + 1);
      uint8_t low = bus_.CpuRead8Bit(addr);
      uint8_t hi;
      if ((addr & 0xFF) == 0xFF) {
        // See https://www.nesdev.org/wiki/Instruction_reference#JMP
        hi = bus_.CpuRead8Bit((addr & 0xFF00));
      } else {
        hi = bus_.CpuRead8Bit(addr + 1);
      }
      result = (hi << 8) | low;
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

void Cpu::UpdateOverflowFlag(uint8_t a, uint8_t b, uint8_t result) {
  // #See https://www.nesdev.org/wiki/Instruction_reference#ADC
  P.OVERFLOW = ((result ^ a) & (result ^ b) & 0x80) != 0;
}

void Cpu::UpdateCarryFlag(int16_t result) {
  P.CARRY = 0;

  if ((result >> 8) != 0) {
    P.CARRY = 1;
  }
}

void Cpu::BranchIf(AddressingMode addressing, bool condition) {
  if (condition) {
    uint16_t old_pc = PC;
    PC = GetAddress(addressing);
    cycles++;

    if (IsCrossPage(old_pc, PC + 2)) {
      cycles++;
    }
  }
}

void Cpu::Compare(AddressingMode addressing, uint8_t reg) {
  uint16_t addr = GetAddress(addressing);
  uint8_t m = bus_.CpuRead8Bit(addr);

  uint8_t result = reg - m;

  UpdateZeroAndNegativeFlag(result);
  P.CARRY = (reg >= m);
}

void Cpu::Increment(uint8_t *target, int value) {
  *target += value;

  UpdateZeroAndNegativeFlag(*target);
}

// Instructions
void Cpu::ADC(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);
  uint8_t pre_a = A;
  uint8_t m = bus_.CpuRead8Bit(addr);
  int16_t tmp_result = A + m + P.CARRY;
  A = tmp_result;

  UpdateZeroAndNegativeFlag(A);
  UpdateOverflowFlag(pre_a, m, A);
  UpdateCarryFlag(tmp_result);
}

void Cpu::AND(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);
  uint8_t m = bus_.CpuRead8Bit(addr);

  A &= m;

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::ASL(AddressingMode addressing) {
  int16_t target;
  if (addressing == kImplicit) {
    target = A;
    target <<= 1;
    A = target;
  } else {
    uint16_t addr = GetAddress(addressing);
    target = bus_.CpuRead8Bit(addr);
    target <<= 1;
    bus_.CpuWrite8Bit(addr, target);
  }

  UpdateZeroAndNegativeFlag(target);
  UpdateCarryFlag(target);
}

void Cpu::BCC(AddressingMode addressing) {
  BranchIf(addressing, (P.CARRY == 0));
}

void Cpu::BCS(AddressingMode addressing) {
  BranchIf(addressing, (P.CARRY == 1));
}

void Cpu::BEQ(AddressingMode addressing) {
  BranchIf(addressing, (P.ZERO == 1));
}

void Cpu::BIT(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);

  uint8_t m = bus_.CpuRead8Bit(addr);

  uint8_t v = A & m;

  UpdateZeroAndNegativeFlag(v);

  Status tmp;
  tmp.raw = m;

  P.OVERFLOW = tmp.OVERFLOW;
}

void Cpu::BMI(AddressingMode addressing) {
  BranchIf(addressing, (P.NEGATIVE == 1));
}

void Cpu::BNE(AddressingMode addressing) {
  BranchIf(addressing, (P.ZERO == 0));
}

void Cpu::BPL(AddressingMode addressing) {
  BranchIf(addressing, (P.NEGATIVE == 0));
}

void Cpu::BVC(AddressingMode addressing) {
  BranchIf(addressing, (P.OVERFLOW == 0));
}

void Cpu::BVS(AddressingMode addressing) {
  BranchIf(addressing, (P.OVERFLOW == 1));
}

void Cpu::BRK(AddressingMode addressing) {
  (void) addressing;

  uint16_t new_pc = PC + 2;
  Push((new_pc & 0xFF));  // low bytes
  Push(new_pc >> 8);  // high bytes
  Status tmp;
  tmp = P;
  tmp.B = 1;
  tmp.UNUSED = 1;
  Push(tmp.raw);
  PC = bus_.CpuRead16Bit(0xFFFE);

  P.INTERRUPT_DISABLE = 1;

  jumped_ = true;
}

void Cpu::CLC(AddressingMode addressing) {
  (void) addressing;
  P.CARRY = 0;
}

void Cpu::CLD(AddressingMode addressing) {
  (void) addressing;
  P.DECIMAL = 0;
}

void Cpu::CLI(AddressingMode addressing) {
  (void) addressing;
  P.INTERRUPT_DISABLE = 0;
}

void Cpu::CLV(AddressingMode addressing) {
  (void) addressing;
  P.OVERFLOW = 0;
}

void Cpu::CMP(AddressingMode addressing) {
  Compare(addressing, A);
}

void Cpu::CPX(AddressingMode addressing) {
  Compare(addressing, X);
}

void Cpu::CPY(AddressingMode addressing) {
  Compare(addressing, Y);
}

void Cpu::DEC(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);
  uint8_t m = bus_.CpuRead8Bit(addr);
  Increment(&m, -1);
  bus_.CpuWrite8Bit(addr, m);
}

void Cpu::DEX(AddressingMode addressing) {
  Increment(&X, -1);
}

void Cpu::DEY(AddressingMode addressing) {
  Increment(&Y, -1);
}

void Cpu::EOR(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);

  uint8_t m = bus_.CpuRead8Bit(addr);

  A ^= m;

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::INC(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);
  uint8_t m = bus_.CpuRead8Bit(addr);
  Increment(&m, 1);
  bus_.CpuWrite8Bit(addr, m);
}

void Cpu::INX(AddressingMode addressing) {
  Increment(&X, 1);
}

void Cpu::INY(AddressingMode addressing) {
  Increment(&Y, 1);
}

void Cpu::JMP(AddressingMode addressing) {
  /*
    Unfortunately, because of a CPU bug, if this 2-byte variable has an address ending in $FF and thus crosses a page, then the CPU fails to increment the page when reading the second byte and thus reads the wrong address. For example, JMP ($03FF) reads $03FF and $0300 instead of $0400. Care should be taken to ensure this variable does not cross a page.
   */
  PC = GetAddress(addressing);
  jumped_ = true;
}

void Cpu::JSR(AddressingMode addressing) {
  uint16_t new_pc = GetAddress(addressing);
  uint16_t next_pc = PC + 2;

  Push((next_pc & 0xFF));
  Push((next_pc >> 8));

  PC = new_pc;

  jumped_ = true;
}

void Cpu::LDA(AddressingMode addressing) {
  LoadToReg(A, addressing);
}

void Cpu::LDX(AddressingMode addressing) {
  LoadToReg(X, addressing);
}

void Cpu::LDY(AddressingMode addressing) {
  LoadToReg(Y, addressing);
}

void Cpu::LSR(AddressingMode addressing) {
  uint8_t target;

  auto func = [this, &target](uint8_t *v){
    P.CARRY = (*v & 0x01);
    *v >>= 1;
    target = *v;
  };

  if (addressing == kImplicit) {
    func(&A);
  } else {
    uint16_t addr = GetAddress(addressing);
    uint8_t m = bus_.CpuRead8Bit(addr);
    func(&m);
    bus_.CpuWrite8Bit(addr, m);
  }

  UpdateZeroAndNegativeFlag(target);
}

void Cpu::NOP(AddressingMode addressing) {
  (void) addressing;
}

void Cpu::ORA(AddressingMode addressing) {
  uint16_t addr = GetAddress(addressing);
  uint8_t m = bus_.CpuRead8Bit(addr);
  A |= m;

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::PHA(AddressingMode addressing) {
  (void) addressing;  // Not used
  Push(A);
}

void Cpu::PHP(AddressingMode addressing) {
  (void) addressing;  // Not used
  Push(P.raw);
}

void Cpu::PLA(AddressingMode addressing) {
  (void) addressing;  // Not used
  A = Pop();

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::RTS(AddressingMode addressing) {
  (void) addressing;
  uint8_t hi = Pop();
  uint8_t low = Pop();
  uint16_t new_pc = ((hi << 8) | low);
  PC = new_pc + 1;

  jumped_ = true;
}

void Cpu::SEC(AddressingMode addressing) {
  (void) addressing;
  P.CARRY = 1;
}

void Cpu::SED(AddressingMode addressing) {
  (void) addressing;
  P.DECIMAL = 1;
}

void Cpu::SEI(AddressingMode addressing) {
  (void) addressing;
  P.INTERRUPT_DISABLE = 1;
}

void Cpu::STA(AddressingMode addressing) {
  (void) addressing;
  StoreToMem(A, addressing);
}

void Cpu::STX(AddressingMode addressing) {
  (void) addressing;
  StoreToMem(X, addressing);
}

void Cpu::STY(AddressingMode addressing) {
  (void) addressing;
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

void Cpu::TSX(AddressingMode addressing) {
  (void) addressing;  // Because we was implied

  Transfer(SP, X);
}

void Cpu::TXA(AddressingMode addressing) {
  (void) addressing;  // Because we was implied

  Transfer(X, A);
}

void Cpu::TXS(AddressingMode addressing) {
  (void) addressing;  // Because we was implied

  Transfer(X, SP, false);
}

void Cpu::TYA(AddressingMode addressing) {
  (void) addressing;  // Because we was implied

  Transfer(Y, A);
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

void Cpu::Push(uint8_t value) {
  bus_.CpuWrite8Bit(0x100 + (SP--), value);
}

uint8_t Cpu::Pop() {
  return bus_.CpuRead8Bit(0x100 + (++SP));
}

}  // namespace nes
