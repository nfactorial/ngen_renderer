add_library(ngen_display STATIC
    source/display_port.cpp
)
add_library(ngen::display ALIAS ngen_display)

target_include_directories(ngen_display PUBLIC include)
