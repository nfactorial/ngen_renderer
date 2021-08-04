
#include <cstdio>
#include <SDL_vulkan.h>
#include "window_surface.h"
#include "vulkan_error.h"

namespace ngen::vulkan {
    WindowSurface::WindowSurface()
    : m_surface(VK_NULL_HANDLE)
    , m_instance(VK_NULL_HANDLE)
    , m_height(0)
    , m_width(0) {
    }

    WindowSurface::~WindowSurface() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void WindowSurface::dispose() {
        if (m_surface != VK_NULL_HANDLE) {
            m_width = 0;
            m_height = 0;

            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
            m_instance = VK_NULL_HANDLE;
        }
    }

    //! \brief Prepares the object for use by the application.
    //! \param context [in] - The VulkanContext to be used.
    //! \param hwnd [in] - The handle to the system window used for presentation.
    //! \returns True if the object initialized successfully otherwise false.
    bool WindowSurface::initialize(VkInstance instance, SDL_Window *window) {
        // ngen::vulkan::platform::initializeSurfaceCreateInfo(createInfo, window->)

        if (!ngen::vulkan::platform::createSurface(window, instance, &m_surface)) {
            printf("Failed to create window surface\n");
            return false;
        }

        // TODO: This makes us hard-coded to SDL, must be removed in the future!
        SDL_Vulkan_GetDrawableSize(window, &m_width, &m_height);

        m_instance = instance;

        printf("Created window surface\n");
        return true;
    }

    //! \brief Event handler invoked when the window has been resized.
    //! \returns <em>True</em> if the resize was handled successfully otherwise <em>false</em>.
    bool WindowSurface::onWindowResized() {
        return false;
    }
}
