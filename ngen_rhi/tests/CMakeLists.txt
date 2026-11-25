add_executable(ngen_rhi_tests
    descriptions_tests.cpp
    fake_device_tests.cpp
    handle_tests.cpp
)

target_link_libraries(ngen_rhi_tests PRIVATE
    ngen::rhi
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ngen_rhi_tests)
