// Spec: rhi-vulkan. Behaviour changes go through it.
#include "surface.h"

#include "vulkan_result.h"

#include <string>
#include <variant>

namespace ngen::rhi::vulkan
{
    namespace
    {
        std::unexpected<ngen::Error> unsupportedKind(const char *const kind)
        {
            return std::unexpected(makeError(ErrorCode::Unsupported, std::string("a NativeWindow holding ") + kind +
                                                                         " is not a window kind this platform uses"));
        }

        const char *kindName(const ngen::platform::NativeWindow &window)
        {
            if (std::holds_alternative<ngen::platform::Win32Handles>(window))
            {
                return "Win32Handles";
            }
            if (std::holds_alternative<ngen::platform::AndroidHandles>(window))
            {
                return "AndroidHandles";
            }
            return "AppleHandles";
        }
    } // namespace

    const char *platformSurfaceExtension()
    {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
        return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#else
        return nullptr;
#endif
    }

    bool platformCanPresent([[maybe_unused]] const VkPhysicalDevice physicalDevice,
                            [[maybe_unused]] const uint32_t queueFamily)
    {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamily) == VK_TRUE;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR) || defined(VK_USE_PLATFORM_METAL_EXT)
        return true;
#else
        return false;
#endif
    }

    std::expected<VkSurfaceKHR, ngen::Error> createSurface([[maybe_unused]] const VkInstance instance,
                                                           const ngen::platform::NativeWindow &window)
    {
        [[maybe_unused]] VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        const auto *handles = std::get_if<ngen::platform::Win32Handles>(&window);
        if (handles == nullptr)
        {
            return unsupportedKind(kindName(window));
        }
        VkWin32SurfaceCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        info.hinstance = static_cast<HINSTANCE>(handles->hinstance);
        info.hwnd = static_cast<HWND>(handles->hwnd);
        const VkResult result = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        const auto *handles = std::get_if<ngen::platform::AndroidHandles>(&window);
        if (handles == nullptr)
        {
            return unsupportedKind(kindName(window));
        }
        VkAndroidSurfaceCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        info.window = static_cast<ANativeWindow *>(handles->nativeWindow);
        const VkResult result = vkCreateAndroidSurfaceKHR(instance, &info, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
        const auto *handles = std::get_if<ngen::platform::AppleHandles>(&window);
        if (handles == nullptr)
        {
            return unsupportedKind(kindName(window));
        }
        VkMetalSurfaceCreateInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        info.pLayer = static_cast<const CAMetalLayer *>(handles->metalLayer);
        const VkResult result = vkCreateMetalSurfaceEXT(instance, &info, nullptr, &surface);
#else
        return unsupportedKind(kindName(window));
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR) || defined(VK_USE_PLATFORM_ANDROID_KHR) || defined(VK_USE_PLATFORM_METAL_EXT)
        if (result != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("creating the window surface", result));
        }
        return surface;
#endif
    }
} // namespace ngen::rhi::vulkan
