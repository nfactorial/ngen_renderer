
#ifndef NGEN_RENDERER_PLATFORM_APPLE
#define NGEN_RENDERER_PLATFORM_APPLE

#include <MoltenVK/vk_mvk_moltenvk.h>
#include <MoltenVK/mvk_vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <SDL_vulkan.h>

namespace ngen::vulkan {
    using PlatformWindow = const void*;
    using SurfaceCreateInfo = VkMacOSSurfaceCreateInfoMVK;

    namespace platform {
        extern const uint32_t kDefaultVulkanExtensionCount;
        extern const char* kDefaultVulkanExtensions[];
        extern const uint32_t kDefaultDeviceExtensionCount;
        extern const char *kDefaultDeviceExtensions[];

        inline void initializeSurfaceCreateInfo(SurfaceCreateInfo &createInfo, PlatformWindow platformWindow) {
            createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.pView = platformWindow;
        }

        inline bool createSurface(SDL_Window *window, VkInstance instance, VkSurfaceKHR* pSurface) {
            // TODO: This locks us to SDL, this should be made framework neutral in the future
            return SDL_Vulkan_CreateSurface(window, instance, pSurface);
            // return vkCreateMacOSSurfaceMVK(instance, &createInfo, pAllocator, pSurface);

            // For iOS...
            // return vkCreateIOSSurfaceMVK(instance, &createInfo, pAllocator, pSurface);
        }
    }
}


#endif //ndef NGEN_RENDERER_PLATFORM_APPLE
