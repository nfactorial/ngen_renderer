add_library(ngen_vulkan STATIC
    source/buffer.cpp
    source/command_pool.cpp
    source/device.cpp
    source/fence.cpp
    source/physical_device.cpp
    source/pipeline.cpp
    source/pipeline_description.cpp
    source/platform.cpp
    source/render_pass.cpp
    source/renderer.cpp
    source/semaphore.cpp
    source/shader.cpp
    source/swap_chain.cpp
    source/vulkan_context.cpp
    source/vulkan_error.cpp
    source/window_surface.cpp
)
add_library(ngen::vulkan ALIAS ngen_vulkan)

target_include_directories(ngen_vulkan PUBLIC include)

# SDL and Vulkan both appear in the public headers, so consumers need them too.
target_link_libraries(ngen_vulkan PUBLIC
    Vulkan::Vulkan
    ${NGEN_SDL2_TARGET}
)

if (WIN32)
    # platform_windows.h includes <windows.h> from a public header.
    target_compile_definitions(ngen_vulkan PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
