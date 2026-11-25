cmake_minimum_required(VERSION 3.24)

project(ngen_renderer LANGUAGES CXX)

option(NGEN_BUILD_EXAMPLE   "Build the example application." ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Vulkan comes from the LunarG SDK; FindVulkan locates it through the VULKAN_SDK environment variable.
find_package(Vulkan REQUIRED)

include(cmake/dependencies.cmake)

add_subdirectory(ngen_vulkan)

if (NGEN_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
