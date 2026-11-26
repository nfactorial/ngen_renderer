// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>

#include <vulkan/vulkan.h>

#include <string_view>

namespace ngen::rhi::vulkan
{
    // The enumerator's name, e.g. "VK_ERROR_DEVICE_LOST".
    [[nodiscard]] const char *resultName(VkResult result);

    // An error for a failed Vulkan call: "<what> failed: <result name>". DEVICE_LOST becomes DeviceLost,
    // SURFACE_LOST SurfaceLost, the out-of-memory results OutOfMemory, the missing driver, layer, extension,
    // feature and format results Unsupported, and anything else Internal.
    [[nodiscard]] ngen::Error vulkanError(std::string_view what, VkResult result);
} // namespace ngen::rhi::vulkan
