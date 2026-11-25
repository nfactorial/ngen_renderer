
#ifndef NGEN_RENDERER_PLATFORM_WINDOWS
#define NGEN_RENDERER_PLATFORM_WINDOWS

#define VK_USE_PLATFORM_WIN32_KHR

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <SDL_vulkan.h>

struct SDL_Window;

namespace ngen::vulkan {
    namespace platform {
        extern const uint32_t kDefaultDeviceExtensionCount;
        extern const char *kDefaultDeviceExtensions[];

        inline bool createSurface(SDL_Window *window, VkInstance instance, VkSurfaceKHR* pSurface) {
            return SDL_Vulkan_CreateSurface(window, instance, pSurface);
        }
    }
}


#endif //ndef NGEN_RENDERER_PLATFORM_WINDOWS
