
#ifndef NGEN_RENDERER_PLATFORM_APPLE
#define NGEN_RENDERER_PLATFORM_APPLE

#define VK_USE_PLATFORM_MACOS_MVK

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>

namespace ngen::vulkan {
    using PlatformWindow = const void*;
    using SurfaceCreateInfo = VkMacOSSurfaceCreateInfoMVK;

    namespace platform {
        extern const uint32_t kDefaultVulkanExtensionCount;
        extern const char* kDefaultkVulkanExtensions[];
        extern const uint32_t kDefaultDeviceExtensionCount;
        extern const char *kDefaultDeviceExtensions[];

        inline void initializeSurfaceCreateInfo(SurfaceCreateInfo &createInfo, PlatformWindow platformWindow) {
            createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.pView = platformWindow;
        }

        inline VkResult createSurface(VkInstance instance, SurfaceCreateInfo &createInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
            return vkCreateMacOSSurfaceMVK(instance, &createInfo, pAllocator, pSurface);
        }
    }
}


#endif //ndef NGEN_RENDERER_PLATFORM_APPLE
