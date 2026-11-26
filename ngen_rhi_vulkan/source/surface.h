// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/platform/window.h>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <expected>

namespace ngen::rhi::vulkan
{
    // The instance extension for the window kind this build's platform uses (VK_KHR_win32_surface,
    // VK_KHR_android_surface or VK_EXT_metal_surface); nullptr on a platform with none.
    [[nodiscard]] const char *platformSurfaceExtension();

    // Whether the platform reports, before any surface exists, that a graphics family can present: on Windows
    // vkGetPhysicalDeviceWin32PresentationSupportKHR, on Android and Apple any graphics family.
    [[nodiscard]] bool platformCanPresent(VkPhysicalDevice physicalDevice, uint32_t queueFamily);

    // Creates a surface from the native handles; Unsupported for a window kind the build's platform does not
    // use.
    [[nodiscard]] std::expected<VkSurfaceKHR, ngen::Error> createSurface(VkInstance instance,
                                                                         const ngen::platform::NativeWindow &window);
} // namespace ngen::rhi::vulkan
