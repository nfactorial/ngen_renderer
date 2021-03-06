cmake_minimum_required(VERSION 3.1)

project(ngen_vulkan)

set(SOURCE_FILES
    source/buffer.cpp
    source/platform.cpp
    source/device.cpp
    source/shader.cpp
    source/fence.cpp
    source/semaphore.cpp
    source/renderer.cpp
    source/swap_chain.cpp
    source/render_pass.cpp
    source/command_pool.cpp
    source/physical_device.cpp
    source/vulkan_error.cpp
    source/window_surface.cpp
    source/pipeline.cpp
    source/pipeline_description.cpp
    source/vulkan_context.cpp)

set(INCLUDE_FILES
    include/buffer.h
    include/platform.h
    include/device.h
    include/shader.h
    include/fence.h
    include/renderer.h
    include/swap_chain.h
    include/semaphore.h
    include/render_pass.h
    include/command_pool.h
    include/physical_device.h
    include/vulkan_error.h
    include/vulkan_error_def.h
    include/window_surface.h
    include/pipeline.h
    include/pipeline_description.h
    include/vertex_attribute.h
    include/vulkan_context.h)

add_definitions(-DNOMINMAX)

find_package(SDL2 REQUIRED)

add_library(ngen_vulkan STATIC
        ${SOURCE_FILES}
        ${INCLUDE_FILES}
        )

add_library(ngen::vulkan ALIAS ngen_vulkan)

target_include_directories(ngen_vulkan PUBLIC
        $<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/ngen/renderer/vulkan>
        )

if(APPLE)
    MESSAGE(STATUS "Using MoltenVK")
    target_include_directories(ngen_vulkan PRIVATE "$ENV{MOLTEN_VK}/include")
    # target_include_directories(ngen_vulkan SYSTEM PRIVATE "~/Library/Frameworks/vulkansdk-macOS-1.1.101.0/MoltenVK/include")
else()
    target_include_directories(ngen_vulkan PRIVATE ${Vulkan_INCLUDE_DIRS})
endif()

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
