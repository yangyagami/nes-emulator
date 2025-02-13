#include "joypad.h"

namespace nes {

Joypad::Joypad() {
  current_key_ = kA;
}

void Joypad::SetKey(Key key, bool pressed) {
  keys_[key] = pressed;
}

bool Joypad::GetCurrentKey() {
  if (strobe_) {
    current_key_ = kA;
    return keys_[kA];
  }

  bool ret = keys_[current_key_];

  if (!strobe_) {
    int tmp = static_cast<int>(current_key_);
    tmp++;
    if (tmp >= 8) {
      current_key_ = kA;
    } else {
      current_key_ = static_cast<Key>(tmp);
    }
  }

  return ret;
}

}  // namespace nes
