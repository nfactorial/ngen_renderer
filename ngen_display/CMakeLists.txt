cmake_minimum_required(VERSION 3.1)

project(ngen_display)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)

set(SOURCE_FILES
        source/display_port.cpp)

set(INCLUDE_FILES
        include/display_port.h)

add_library(ngen_display STATIC
        ${SOURCE_FILES}
        ${INCLUDE_FILES}
        )

add_library(ngen::display ALIAS ngen_display)

target_include_directories(ngen_display PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/ngen/renderer/display>
        )

if (NGEN_BUILD_TESTS)
    # add_subdirectory(test)
endif()
