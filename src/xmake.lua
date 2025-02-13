target("nes")
set_kind("shared")
add_files(
   "cpu/*.cc",
   "bus/*.cc",
   "utils/*.cc",
   "cartridge/*.cc",
   "ppu/*.cc",
   "joypad/*.cc"
)
add_includedirs(".", { public = true })
add_packages("raylib")


target("nes-emulator")
set_kind("binary")
add_deps("nes")
add_files("machine.cc")
add_packages("raylib")
