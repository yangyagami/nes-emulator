target("nes")
set_kind("shared")
add_files("cpu/*.cc", "bus/*.cc", "utils/*.cc")
add_includedirs(".", { public = true })
