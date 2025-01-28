#ifndef NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_
#define NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_

#include <array>
#include <string>
#include <cstdint>
#include <format>

namespace nes {

struct Cartridge {
  // See https://www.nesdev.org/wiki/INES#Flags_6
  union {
    struct {
      uint8_t NAMETABLE_ARRANGEMENT : 1;
      uint8_t PRG_RAM : 1;
      uint8_t TRAINER : 1;
      uint8_t ALTERNATIVE_NAMETABLE : 1;
      uint8_t MAPPER_NUMBER : 4;
    };
    uint8_t raw;
  } Flags6;

 public:
  class NoSuchFileException : public std::exception {
   public:
    NoSuchFileException(const std::string &path) {
      msg_ = std::format("No such file: {}", path);
    }
    const char *what() const noexcept override {
      return msg_.c_str();
    }

   private:
    std::string msg_;
  };

  class InvalidRomException : public std::exception {
   public:
    InvalidRomException() {
      msg_ = "Invalid rom";
    }
    const char *what() const noexcept override {
      return msg_.c_str();
    }

   private:
    std::string msg_;
  };

  Cartridge(const std::string &path, std::array<uint8_t, 0x10000> &memory);
};

}  // namespace nes

#endif  // NES_EMULATOR_CARTRIDGE_CARTRIDGE_H_
