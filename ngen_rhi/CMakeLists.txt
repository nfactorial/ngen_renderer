add_library(ngen_rhi STATIC
    source/descriptions.cpp
    source/device.cpp
    source/fake_device.cpp
)
add_library(ngen::rhi ALIAS ngen_rhi)

target_include_directories(ngen_rhi PUBLIC include)
target_link_libraries(ngen_rhi PUBLIC ngen::platform)

if (NGEN_BUILD_TESTS)
    add_subdirectory(tests)
endif()
