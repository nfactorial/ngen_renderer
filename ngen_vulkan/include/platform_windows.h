
#ifndef NGEN_RENDERER_PLATFORM_WINDOWS
#define NGEN_RENDERER_PLATFORM_WINDOWS

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

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

        inline VkResult createSurface(VkInstance instance, SurfaceCreateInfo &createInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
            return vkCreateWin32SurfaceKHR(instance, &createInfo, pAllocator, pSurface);
        }
    }
}


#endif //ndef NGEN_RENDERER_PLATFORM_WINDOWS
