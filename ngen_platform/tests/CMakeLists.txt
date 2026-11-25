add_executable(ngen_platform_tests
    error_tests.cpp
    fake_window_tests.cpp
    sdl_window_tests.cpp
)

target_link_libraries(ngen_platform_tests PRIVATE
    ngen::platform
    ${NGEN_SDL3_TARGET}
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ngen_platform_tests)
