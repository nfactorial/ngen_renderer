
#include "platform.h"

namespace ngen::vulkan::platform {
#if defined(__APPLE__)
    const uint32_t kDefaultVulkanExtensionCount = 1;
    const char *kDefaultVulkanExtensions[] = {
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
        VK_MVK_MOLTENVK_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
    };

    const uint32_t kDefaultDeviceExtensionCount = 1;
    const char *kDefaultDeviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#elif defined(_WIN32)
    const uint32_t kDefaultVulkanExtensionCount = 2;
    const char *kDefaultVulkanExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };

    const uint32_t kDefaultDeviceExtensionCount = 1;
    const char *kDefaultDeviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#endif
}
