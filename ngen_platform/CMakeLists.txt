add_library(ngen_platform STATIC
    source/error.cpp
    source/fake_window.cpp
    source/sdl_window.cpp
)
add_library(ngen::platform ALIAS ngen_platform)

target_include_directories(ngen_platform PUBLIC include)

# Only sdl_window.h exposes SDL types, so code that includes it links SDL3 itself.
target_link_libraries(ngen_platform PRIVATE ${NGEN_SDL3_TARGET})

if (NGEN_BUILD_TESTS)
    add_subdirectory(tests)
endif()
