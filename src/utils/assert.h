#ifndef NES_EMULATOR_UTILS_ASSERT_H_
#define NES_EMULATOR_UTILS_ASSERT_H_

#include <string>
#include <iostream>
#include <format>

#define nes_assert(condition, msg) \
  do { \
    if (!(condition)) { \
      std::cerr << std::format("[{}:{}][{}]: {}", \
                               __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                               msg) << std::endl;       \
      std::terminate(); \
    } \
  } while (0)

#endif  // NES_EMULATOR_UTILS_ASSERT_H_
