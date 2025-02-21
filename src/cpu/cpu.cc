#include "cpu.h"

#include <format>
#include <iostream>
#include <cassert>
#include <string>

#include "bus/bus.h"
#include "utils/assert.h"

namespace nes {

Cpu::Cpu(Bus &bus) : bus_(bus) {
  nmi_flipflop = false;
}

void Cpu::Tick() {
  nes_assert(cycles == 0, std::format("Invalid cycles: {}", cycles));

  jumped_ = false;

  // See https://www.nesdev.org/wiki/NMI
  if (nmi_flipflop) {
    NMI();
  }

  // std::cout << std::format("PC: {:#x}\n", PC);
  // Fetch opcode
  uint8_t opcode = bus_.CpuRead8Bit(PC);

  nes_assert(kOpcodes.find(opcode) != kOpcodes.end(),
             std::format("Invalid opcode: 0x{:02x}, PC: 0x{:04x}", opcode, PC));

  Opcode opcode_obj = kOpcodes.find(opcode)->second;

  cycles = opcode_obj.cycles;

  opcode_obj.func(opcode_obj);

  if (!jumped_) {
    PC += opcode_obj.bytes;
  }

  if (interrupt_disable_delay_ > 0) {
    interrupt_disable_delay_--;
    if (interrupt_disable_delay_ == 0) {
      P.INTERRUPT_DISABLE = interrupt_disable_latch_;
    }
  }
}

void Cpu::Reset() {
  A = X = Y = 0;
  SP = 0XFF;
  P.raw = 0;
  P.B = 0;
  P.INTERRUPT_DISABLE = 1;
  P.UNUSED = 1;

  nmi_flipflop = false;

  cycles = 0;
}

std::string Cpu::Disassemble(uint16_t address) {
  uint8_t opcode = bus_.CpuRead8Bit(address);

  nes_assert(kOpcodes.find(opcode) != kOpcodes.end(),
             std::format("Invalid opcode: 0x{:02x}, PC: 0x{:04x}", opcode, PC));

  Opcode opcode_obj = kOpcodes.find(opcode)->second;

  std::string right;

  switch (opcode_obj.addressing_mode) {
    case kAbsolute: {
      right = std::format("${:04x}", bus_.CpuRead16Bit(address + 1));
      break;
    }
    case kZeroPage: {
      right = std::format("${:02x}", bus_.CpuRead8Bit(address + 1));
      break;
    }
    case kZeroPageX: {
      right = std::format("${:02x}, X", bus_.CpuRead8Bit(address + 1));
      break;
    }
    case kZeroPageY: {
      right = std::format("${:02x}, Y", bus_.CpuRead8Bit(address + 1));
      break;
    }
    case kAbsoluteX: {
      right = std::format("${:04x}, X", bus_.CpuRead16Bit(address + 1));
      break;
    }
    case kAbsoluteY: {
      right = std::format("${:04x}, Y", bus_.CpuRead16Bit(address + 1));
      break;
    }
    case kImmediate: {
      right = std::format("#${:02x}", bus_.CpuRead8Bit(address + 1));
      break;
    }
    case kRelative: {
      right = std::format("(${:02x})", bus_.CpuRead8Bit(address + 1));
      break;
    }
    case kImplicit: {
      right = "";
      break;
    }
    case kIndirect: {
      right = std::format("(${:04x})", bus_.CpuRead16Bit(address + 1));
      break;
    }
    case kIndexedIndirect: {
      right = std::format("(${:02x}, X)", bus_.CpuRead8Bit(address + 1));
      break;
    }
    case kIndirectIndexed: {
      right = std::format("(${:02x}), Y", bus_.CpuRead8Bit(address + 1));
      break;
    }
    default: {
      right = "UNKNOWN";
      break;
    }
  }

  return std::format("{} {}", opcode_obj.name, right);
}

// Instructions
void Cpu::ADC(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t pre_a = A;
  uint8_t m = bus_.CpuRead8Bit(addr);
  int16_t tmp_result = A + m + P.CARRY;
  A = tmp_result;

  UpdateZeroAndNegativeFlag(A);
  UpdateOverflowFlag(pre_a, m, A);
  UpdateCarryFlag(tmp_result);
}

void Cpu::AND(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t m = bus_.CpuRead8Bit(addr);

  A &= m;

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::ASL(Opcode &opcode_obj) {
  int16_t target;
  AddressingMode addressing = opcode_obj.addressing_mode;
  if (addressing == kImplicit) {
    target = A;
    target <<= 1;
    A = target;
  } else {
    uint16_t addr = GetAddress(opcode_obj);
    target = bus_.CpuRead8Bit(addr);
    target <<= 1;
    bus_.CpuWrite8Bit(addr, target);
  }

  UpdateZeroAndNegativeFlag(target);
  UpdateCarryFlag(target);
}

void Cpu::BCC(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.CARRY == 0));
}

void Cpu::BCS(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.CARRY == 1));
}

void Cpu::BEQ(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.ZERO == 1));
}

void Cpu::BIT(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);

  uint8_t m = bus_.CpuRead8Bit(addr);

  uint8_t v = A & m;

  P.ZERO = (v == 0 ? 1 : 0);

  Status tmp;
  tmp.raw = m;

  P.OVERFLOW = tmp.OVERFLOW;
  P.NEGATIVE = tmp.NEGATIVE;
}

void Cpu::BMI(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.NEGATIVE == 1));
}

void Cpu::BNE(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.ZERO == 0));
}

void Cpu::BPL(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.NEGATIVE == 0));
}

void Cpu::BVC(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.OVERFLOW == 0));
}

void Cpu::BVS(Opcode &opcode_obj) {
  BranchIf(opcode_obj, (P.OVERFLOW == 1));
}

void Cpu::BRK(Opcode &opcode_obj) {
  (void) opcode_obj;

  uint16_t new_pc = PC + 2;
  Push(new_pc >> 8);  // high bytes
  Push((new_pc & 0xFF));  // low bytes
  Status tmp;
  tmp = P;
  tmp.B = 1;
  tmp.UNUSED = 1;
  Push(tmp.raw);
  PC = bus_.CpuRead16Bit(0xFFFE);

  P.INTERRUPT_DISABLE = 1;

  jumped_ = true;
}

void Cpu::CLC(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.CARRY = 0;
}

void Cpu::CLD(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.DECIMAL = 0;
}

void Cpu::CLI(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.INTERRUPT_DISABLE = 0;
}

void Cpu::CLV(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.OVERFLOW = 0;
}

void Cpu::CMP(Opcode &opcode_obj) {
  Compare(opcode_obj, A);
}

void Cpu::CPX(Opcode &opcode_obj) {
  Compare(opcode_obj, X);
}

void Cpu::CPY(Opcode &opcode_obj) {
  Compare(opcode_obj, Y);
}

void Cpu::DEC(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t m = bus_.CpuRead8Bit(addr);
  Increment(&m, -1);
  bus_.CpuWrite8Bit(addr, m);
}

void Cpu::DEX(Opcode &opcode_obj) {
  (void) opcode_obj;
  Increment(&X, -1);
}

void Cpu::DEY(Opcode &opcode_obj) {
  (void) opcode_obj;
  Increment(&Y, -1);
}

void Cpu::EOR(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);

  uint8_t m = bus_.CpuRead8Bit(addr);

  A ^= m;

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::INC(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t m = bus_.CpuRead8Bit(addr);
  Increment(&m, 1);
  bus_.CpuWrite8Bit(addr, m);
}

void Cpu::INX(Opcode &opcode_obj) {
  (void) opcode_obj;
  Increment(&X, 1);
}

void Cpu::INY(Opcode &opcode_obj) {
  (void) opcode_obj;
  Increment(&Y, 1);
}

void Cpu::JMP(Opcode &opcode_obj) {
  /*
    Unfortunately, because of a CPU bug, if this 2-byte variable has an address ending in $FF and thus crosses a page, then the CPU fails to increment the page when reading the second byte and thus reads the wrong address. For example, JMP ($03FF) reads $03FF and $0300 instead of $0400. Care should be taken to ensure this variable does not cross a page.
   */
  PC = GetAddress(opcode_obj);
  jumped_ = true;
}

void Cpu::JSR(Opcode &opcode_obj) {
  uint16_t new_pc = GetAddress(opcode_obj);
  uint16_t next_pc = PC + 2;

  Push((next_pc >> 8));
  Push((next_pc & 0xFF));

  PC = new_pc;

  jumped_ = true;
}

void Cpu::LDA(Opcode &opcode_obj) {
  LoadToReg(A, opcode_obj);
}

void Cpu::LDX(Opcode &opcode_obj) {
  LoadToReg(X, opcode_obj);
}

void Cpu::LDY(Opcode &opcode_obj) {
  LoadToReg(Y, opcode_obj);
}

void Cpu::LSR(Opcode &opcode_obj) {
  uint8_t target;

  auto func = [this, &target](uint8_t *v){
    P.CARRY = (*v & 0x01);
    *v >>= 1;
    target = *v;
  };

  if (opcode_obj.addressing_mode == kImplicit) {
    func(&A);
  } else {
    uint16_t addr = GetAddress(opcode_obj);
    uint8_t m = bus_.CpuRead8Bit(addr);
    func(&m);
    bus_.CpuWrite8Bit(addr, m);
  }

  UpdateZeroAndNegativeFlag(target);
}

void Cpu::NMI() {
  uint16_t new_pc = PC;

  Push(new_pc >> 8);  // high bytes
  Push((new_pc & 0xFF));  // low bytes

  Status tmp;
  tmp = P;
  tmp.B = 1;
  tmp.UNUSED = 1;
  Push(tmp.raw);

  PC = bus_.CpuRead16Bit(0xFFFA);
  nmi_flipflop = false;

  // P.INTERRUPT_DISABLE = 1;
}

void Cpu::NOP(Opcode &opcode_obj) {
  (void) opcode_obj;
}

void Cpu::ORA(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t m = bus_.CpuRead8Bit(addr);
  A |= m;

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::PHA(Opcode &opcode_obj) {
  (void) opcode_obj;  // Not used
  Push(A);
}

void Cpu::PHP(Opcode &opcode_obj) {
  (void) opcode_obj;  // Not used
  Status tmp = P;
  tmp.B = 1;
  tmp.UNUSED = 1;
  Push(tmp.raw);
}

void Cpu::PLA(Opcode &opcode_obj) {
  (void) opcode_obj;  // Not used
  A = Pop();

  UpdateZeroAndNegativeFlag(A);
}

void Cpu::PLP(Opcode &opcode_obj) {
  (void) opcode_obj;  // Not used
  Status tmp;
  tmp.raw = Pop();

  P.CARRY = tmp.CARRY;
  P.ZERO = tmp.ZERO;
  // P.INTERRUPT_DISABLE = tmp.INTERRUPT_DISABLE; delayed 1 instruction.
  interrupt_disable_latch_ = tmp.INTERRUPT_DISABLE;
  interrupt_disable_delay_ = 2;
  P.DECIMAL = tmp.DECIMAL;
  P.OVERFLOW = tmp.OVERFLOW;
  P.NEGATIVE = tmp.NEGATIVE;
}

void Cpu::ROL(Opcode &opcode_obj) {
  auto func = [this](uint8_t *v) {
    int16_t tmp = *v << 1;
    tmp = (tmp & ~1) | (P.CARRY & 1);
    P.CARRY = tmp >> 8;
    *v = tmp;

    UpdateZeroAndNegativeFlag(*v);
  };

  if (opcode_obj.addressing_mode == kImplicit) {
    func(&A);
  } else {
    uint16_t addr = GetAddress(opcode_obj);
    uint8_t m = bus_.CpuRead8Bit(addr);
    func(&m);
    bus_.CpuWrite8Bit(addr, m);
  }
}

void Cpu::ROR(Opcode &opcode_obj) {
  auto func = [this](uint8_t *v) {
    uint8_t bit_zero = *v & 1;
    *v >>= 1;
    *v |= (P.CARRY << 7);
    P.CARRY = bit_zero;

    UpdateZeroAndNegativeFlag(*v);
  };

  if (opcode_obj.addressing_mode == kImplicit) {
    func(&A);
  } else {
    uint16_t addr = GetAddress(opcode_obj);
    uint8_t m = bus_.CpuRead8Bit(addr);
    func(&m);
    bus_.CpuWrite8Bit(addr, m);
  }
}

void Cpu::RTI(Opcode &opcode_obj) {
  (void) opcode_obj;

  Status old = P;
  P.raw = Pop();
  P.UNUSED = old.UNUSED;
  P.B = old.B;

  uint8_t low = Pop();
  uint8_t hi = Pop();

  PC = ((hi << 8) | low);

  jumped_ = true;
}

void Cpu::RTS(Opcode &opcode_obj) {
  (void) opcode_obj;
  uint8_t low = Pop();
  uint8_t hi = Pop();
  uint16_t new_pc = ((hi << 8) | low);
  PC = new_pc + 1;

  jumped_ = true;
}

void Cpu::SEC(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.CARRY = 1;
}

void Cpu::SED(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.DECIMAL = 1;
}

void Cpu::SEI(Opcode &opcode_obj) {
  (void) opcode_obj;
  P.INTERRUPT_DISABLE = 1;
}

void Cpu::STA(Opcode &opcode_obj) {
  StoreToMem(A, opcode_obj);
}

void Cpu::STX(Opcode &opcode_obj) {
  StoreToMem(X, opcode_obj);
}

void Cpu::STY(Opcode &opcode_obj) {
  StoreToMem(Y, opcode_obj);
}

void Cpu::SBC(Opcode &opcode_obj) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t m = bus_.CpuRead8Bit(addr);

  uint8_t pre_a = A;
  int16_t result = A - m - ((~P.CARRY) & 0x01);
  A = result;

  UpdateZeroAndNegativeFlag(A);
  UpdateOverflowFlag(pre_a, ~m, A);
  P.CARRY = ~static_cast<uint8_t>(result < 0x00);
}

void Cpu::TAX(Opcode &opcode_obj) {
  (void) opcode_obj;  // Because we was implied

  Transfer(A, X);
}

void Cpu::TAY(Opcode &opcode_obj) {
  (void) opcode_obj;  // Because we was implied

  Transfer(A, Y);
}

void Cpu::TSX(Opcode &opcode_obj) {
  (void) opcode_obj;  // Because we was implied

  Transfer(SP, X);
}

void Cpu::TXA(Opcode &opcode_obj) {
  (void) opcode_obj;  // Because we was implied

  Transfer(X, A);
}

void Cpu::TXS(Opcode &opcode_obj) {
  (void) opcode_obj;  // Because we was implied

  Transfer(X, SP, false);
}

void Cpu::TYA(Opcode &opcode_obj) {
  (void) opcode_obj;  // Because we was implied

  Transfer(Y, A);
}

uint16_t Cpu::GetAddress(Opcode &opcode_obj) {
  uint16_t result = 0;

  AddressingMode addressing_mode = opcode_obj.addressing_mode;

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
      result = AbsoluteAdd(X, opcode_obj.cycles_plus);
      break;
    }
    case kAbsoluteY: {
      result = AbsoluteAdd(Y, opcode_obj.cycles_plus);
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

      uint8_t low = bus_.CpuRead8Bit(tmp);
      uint8_t hi;

      if (tmp == 0xFF) {
        hi = bus_.CpuRead8Bit(0x0);
      } else {
        hi = bus_.CpuRead8Bit(tmp + 1);
      }

      result = (hi << 8) | low;
      break;
    }
    case kIndirectIndexed: {
      uint16_t addr = (PC + 1);
      uint8_t zp_addr = bus_.CpuRead8Bit(addr);
      uint16_t indirect;
      if (zp_addr == 0xFF) {
        indirect = (bus_.CpuRead8Bit(0x00) << 8) | bus_.CpuRead8Bit(zp_addr);
      } else {
        uint8_t zp_addr = bus_.CpuRead8Bit(addr);
        indirect = bus_.CpuRead16Bit(zp_addr);
      }

      result = indirect + Y;

      if (opcode_obj.cycles_plus && IsCrossPage(indirect, result)) {
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

bool Cpu::IsCrossPage(uint16_t old_address, uint16_t new_address) {
  const int kPageSize = 256;
  if (old_address / kPageSize == new_address / kPageSize) {
    return false;
  }
  return true;
}

uint16_t Cpu::AbsoluteAdd(uint8_t reg, bool cycles_plus) {
  uint16_t result = 0;

  uint16_t before = bus_.CpuRead16Bit(PC + 1);
  result = before + reg;

  if (cycles_plus && IsCrossPage(before, result)) {
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

void Cpu::LoadToReg(uint8_t &reg, Opcode &opcode_obj) {
  uint16_t new_address = GetAddress(opcode_obj);
  reg = bus_.CpuRead8Bit(new_address);

  UpdateZeroAndNegativeFlag(reg);
}

void Cpu::StoreToMem(uint8_t reg, Opcode &opcode_obj) {
  uint16_t new_address = GetAddress(opcode_obj);
  bus_.CpuWrite8Bit(new_address, reg);
}

void Cpu::Transfer(uint8_t from, uint8_t &to, bool p) {
  to = from;

  if (p) {
    UpdateZeroAndNegativeFlag(to);
  }
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

void Cpu::BranchIf(Opcode &opcode_obj, bool condition) {
  if (condition) {
    uint16_t old_pc = PC + 2;
    PC = GetAddress(opcode_obj);
    cycles++;

    if (opcode_obj.cycles_plus && IsCrossPage(old_pc, PC + 2)) {
      cycles++;
    }
  }
}

void Cpu::Compare(Opcode &opcode_obj, uint8_t reg) {
  uint16_t addr = GetAddress(opcode_obj);
  uint8_t m = bus_.CpuRead8Bit(addr);

  uint8_t result = reg - m;

  UpdateZeroAndNegativeFlag(result);
  P.CARRY = (reg >= m);
}

void Cpu::Increment(uint8_t *target, int value) {
  *target += value;

  UpdateZeroAndNegativeFlag(*target);
}

void Cpu::Push(uint8_t value) {
  bus_.CpuWrite8Bit(0x100 + SP, value);
  SP--;
}

uint8_t Cpu::Pop() {
  SP++;
  return bus_.CpuRead8Bit(0x100 + SP);
}

}  // namespace nes
