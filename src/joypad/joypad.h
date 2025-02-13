#ifndef NES_EMULATOR_JOYPAD_JOYPAD_H_
#define NES_EMULATOR_JOYPAD_JOYPAD_H_

#include <array>

namespace nes {

class Joypad {
 public:
  enum Key {
    kA = 0,
    kB,
    kSelect,
    kStart,
    kUp,
    kDown,
    kLeft,
    kRight,
  };


  Joypad();

  void set_strobe(bool flag) { strobe_ = flag; }

  void SetKey(Key key, bool pressed);
  bool GetCurrentKey();

 private:
  /*
    While S (strobe) is high, the shift registers in the controllers are continuously reloaded from the button states, and reading $4016/$4017 will keep returning the current state of the first button (A). Once S goes low, this reloading will stop. Hence a 1/0 write sequence is required to get the button states, after which the buttons can be read back one at a time.
   */
  bool strobe_ = true;

  Key current_key_;

  std::array<bool, 8> keys_;
};

}  // namespace nes

#endif  // NES_EMULATOR_JOYPAD_JOYPAD_H_
