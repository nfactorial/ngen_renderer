
#include "platform.h"

namespace ngen::vulkan::platform {
    // Instance extensions come from SDL_Vulkan_GetInstanceExtensions, so only device extensions are listed here.
    const char *kDefaultDeviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const uint32_t kDefaultDeviceExtensionCount = sizeof(kDefaultDeviceExtensions) / sizeof(kDefaultDeviceExtensions[0]);
}
