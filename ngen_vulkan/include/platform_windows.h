
#ifndef NGEN_RENDERER_PLATFORM_WINDOWS
#define NGEN_RENDERER_PLATFORM_WINDOWS

#define VK_USE_PLATFORM_WIN32_KHR

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <SDL_vulkan.h>

struct SDL_Window;

namespace ngen::vulkan {
    using PlatformWindow = HWND;
    using SurfaceCreateInfo = VkWin32SurfaceCreateInfoKHR;

    namespace platform {
        extern const uint32_t kDefaultVulkanExtensionCount;
        extern const char* kDefaultVulkanExtensions[];
        extern const uint32_t kDefaultDeviceExtensionCount;
        extern const char *kDefaultDeviceExtensions[];

        inline void initializeSurfaceCreateInfo(SurfaceCreateInfo &createInfo, PlatformWindow platformWindow) {
            createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.hwnd = platformWindow;
            createInfo.hinstance = GetModuleHandle(nullptr);
        }

        inline bool createSurface(SDL_Window *window, VkInstance instance, VkSurfaceKHR* pSurface) {
            return SDL_Vulkan_CreateSurface(window, instance, pSurface);
        }
    }
}


#endif //ndef NGEN_RENDERER_PLATFORM_WINDOWS
