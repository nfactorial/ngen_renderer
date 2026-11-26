cmake_minimum_required(VERSION 3.24)

project(ngen_renderer LANGUAGES CXX)

option(NGEN_BUILD_EXAMPLE   "Build the example application." ON)
option(NGEN_BUILD_TESTS     "Build the unit tests." ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Vulkan comes from the LunarG SDK; FindVulkan locates it through the VULKAN_SDK environment variable.
find_package(Vulkan REQUIRED)

include(cmake/dependencies.cmake)

if (NGEN_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(ngen_platform)
add_subdirectory(ngen_rhi)
add_subdirectory(ngen_rhi_vulkan)

if (NGEN_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
