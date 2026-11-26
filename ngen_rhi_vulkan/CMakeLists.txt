add_library(ngen_rhi_vulkan STATIC
    source/buffer.cpp
    source/command_list.cpp
    source/device_impl.cpp
    source/device_selection.cpp
    source/pipeline.cpp
    source/render_pass_cache.cpp
    source/surface.cpp
    source/swap_chain.cpp
    source/vma.cpp
    source/vulkan_context.cpp
    source/vulkan_device.cpp
    source/vulkan_result.cpp
)
add_library(ngen::rhi_vulkan ALIAS ngen_rhi_vulkan)

target_include_directories(ngen_rhi_vulkan PUBLIC include)

# Vulkan and VMA appear only in the sources, never in the public header.
target_link_libraries(ngen_rhi_vulkan
    PUBLIC
        ngen::rhi
        ngen::platform
    PRIVATE
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)

# The surface extension the build's platform uses.
if (WIN32)
    target_compile_definitions(ngen_rhi_vulkan PRIVATE VK_USE_PLATFORM_WIN32_KHR NOMINMAX WIN32_LEAN_AND_MEAN)
elseif (ANDROID)
    target_compile_definitions(ngen_rhi_vulkan PRIVATE VK_USE_PLATFORM_ANDROID_KHR)
elseif (APPLE)
    target_compile_definitions(ngen_rhi_vulkan PRIVATE VK_USE_PLATFORM_METAL_EXT)
endif()

if (NGEN_BUILD_TESTS)
    add_subdirectory(tests)
endif()
