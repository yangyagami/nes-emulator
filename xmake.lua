add_rules("mode.check", "mode.debug", "mode.release")

add_cxflags("-Wall", "-Werror", "-Wextra", "-std=c++20")

add_requires("raylib")

includes("src", "test")

