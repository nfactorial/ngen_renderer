
#ifndef NGEN_RENDERER_PLATFORM_APPLE
#define NGEN_RENDERER_PLATFORM_APPLE

// VK_KHR_portability_subset is still a provisional (beta) extension.
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

namespace ngen::vulkan {
    namespace platform {
        extern const uint32_t kDefaultDeviceExtensionCount;
        extern const char *kDefaultDeviceExtensions[];

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
