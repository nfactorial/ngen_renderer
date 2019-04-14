
#include <cstdio>
#include "window_surface.h"
#include "vulkan_error.h"

namespace ngen::vulkan {
    WindowSurface::WindowSurface()
    : m_surface(VK_NULL_HANDLE) {
    }

    WindowSurface::~WindowSurface() {
    }

    //! \brief Releases all resources currently referenced by this object.
    void WindowSurface::dispose(VkInstance instance) {
        if (m_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }
    }

    //! \brief Prepares the object for use by the application.
    //! \param context [in] - The VulkanContext to be used.
    //! \param hwnd [in] - The handle to the system window used for presentation.
    //! \returns True if the object initialized successfully otherwise false.
    bool WindowSurface::initialize(VkInstance instance, PlatformWindow platformWindow) {
        SurfaceCreateInfo createInfo;

        ngen::vulkan::platform::initializeSurfaceCreateInfo(createInfo, platformWindow);

        VkResult result = ngen::vulkan::platform::createSurface(instance, createInfo, nullptr, &m_surface);
        if (result != VK_SUCCESS) {
            printf("Failed to create window surface: %s\n", getResultString(result));
            return false;
        }

        printf("Created window surface\n");
        return true;
    }
}
