add_executable(ngen_rhi_vulkan_tests
    choice_tests.cpp
    smoke_tests.cpp
    window_tests.cpp
)

# choice_tests.cpp checks the backend's internal selection rules, so it sees the sources' headers.
target_include_directories(ngen_rhi_vulkan_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../source)

# Only the tests use ngen_platform's SDL implementation (SdlWindow); the library does not.
target_link_libraries(ngen_rhi_vulkan_tests PRIVATE
    ngen::rhi_vulkan
    ngen::platform
    ${NGEN_SDL3_TARGET}
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    GTest::gtest_main
)

# The test's own Slang module, compiled to SPIR-V 1.3 with both entry points (vertexMain, fragmentMain) kept
# by name. slangc ships with the Vulkan SDK.
find_program(SLANGC_EXECUTABLE slangc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" REQUIRED)

set(TEST_SHADER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/test_triangle.slang)
set(TEST_SHADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/test_triangle.spv)
add_custom_command(
    OUTPUT  ${TEST_SHADER_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${SLANGC_EXECUTABLE} ${TEST_SHADER_SOURCE} -target spirv -profile spirv_1_3 -fvk-use-entrypoint-name
            -entry vertexMain -stage vertex -entry fragmentMain -stage fragment -o ${TEST_SHADER_OUTPUT}
    DEPENDS ${TEST_SHADER_SOURCE}
    COMMENT "Compiling test_triangle.slang"
    VERBATIM
)
add_custom_target(ngen_rhi_vulkan_test_shaders DEPENDS ${TEST_SHADER_OUTPUT})
add_dependencies(ngen_rhi_vulkan_tests ngen_rhi_vulkan_test_shaders)

# The tests find the compiled shader through this path.
target_compile_definitions(ngen_rhi_vulkan_tests PRIVATE NGEN_TEST_SHADER_PATH="${TEST_SHADER_OUTPUT}")

include(GoogleTest)

# Tests on a real Vulkan device (the smoke and window tests among them) carry the gpu label.
gtest_discover_tests(ngen_rhi_vulkan_tests
    TEST_FILTER "VulkanDevice*"
    PROPERTIES LABELS gpu
)
gtest_discover_tests(ngen_rhi_vulkan_tests
    TEST_FILTER "-VulkanDevice*"
)
