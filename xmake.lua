toolchain("nixos")
    set_kind("standalone")
    set_toolset("ld", "g++", "gcc")
    set_toolset("cc", "gcc")
    set_toolset("cxx", "gcc", "g++")
    set_toolset("sh", "g++", "gcc")
    set_toolset("ar", "ar")
    set_toolset("ex", "ar")
    set_toolset("strip", "strip")
    set_toolset("mm", "gcc")
    set_toolset("mxx", "gcc", "g++")
    set_toolset("as", "gcc")
toolchain_end()

add_rules("mode.check", "mode.debug", "mode.release")

add_cxflags("-Wall", "-Wextra", "-std=c++20")

add_requires("raylib", "gtest")

includes("src", "test")

