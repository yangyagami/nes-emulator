#include "machine.h"

namespace nes {

Machine::Machine() bus_(memory_, cartridge_) {
}

void Machine::Run() {}

}  // namespace nes
