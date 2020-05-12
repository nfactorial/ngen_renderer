cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

enable_testing()

MESSAGE(STATUS "SDL2DIR: $ENV{SDL2DIR}")
MESSAGE(STATUS "SDL2_PATH: $ENV{SDL2_PATH}")
MESSAGE(STATUS "VULKAN_SDK: $ENV{VULKAN_SDK}")
MESSAGE(STATUS "MOLTEN_VK: $ENV{MOLTEN_VK}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
add_definitions(-DGTEST_LANG_CXX11)

add_subdirectory(external)

option(NGEN_BUILD_TESTS "Build unit tests." ON)

project(ngen_renderer)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library")
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

add_subdirectory(ngen_vulkan)
add_subdirectory(ngen_display)
add_subdirectory(example)
