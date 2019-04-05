cmake_minimum_required(VERSION 3.1)

project(ngen_vulkan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)

set(SOURCE_FILES
    source/physical_device.cpp
    source/vulkan_error.cpp
    source/window_surface.cpp
    source/vulkan_context.cpp)

set(INCLUDE_FILES
    include/physical_device.h
    include/vulkan_error.h
    include/window_surface.h
    include/vulkan_context.h)

add_library(ngen_vulkan STATIC
        ${SOURCE_FILES}
        ${INCLUDE_FILES}
        )

add_library(ngen::vulkan ALIAS ngen_vulkan)

target_include_directories(ngen_vulkan PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/ngen/renderer/vulkan>
        )

target_include_directories(ngen_vulkan PRIVATE ${Vulkan_INCLUDE_DIRS}
        )

install(TARGETS ngen_vulkan EXPORT ngen_vulkan_targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

install(EXPORT ngen_vulkan_targets
        FILE ngen_vulkan_targets.cmake
        NAMESPACE ngen::
        DESTINATION lib/cmake/ngen
        )

if (NGEN_BUILD_TESTS)
    # add_subdirectory(test)
endif()
