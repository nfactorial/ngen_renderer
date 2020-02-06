
#ifndef NGEN_VULKAN_CONTEXT
#define NGEN_VULKAN_CONTEXT

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "physical_device.h"
#include "window_surface.h"
#include "swap_chain.h"
#include "device.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;

    class VulkanContext {
    public:
        VulkanContext();
        ~VulkanContext();

        void dispose();
        bool initialize(SDL_Window *window, const char *applicationName);

        bool onWindowResized();

        operator VkInstance() const; // NOLINT

        [[nodiscard]] Device& getDevice();
        [[nodiscard]] const Device& getDevice() const;

        static void dumpExtensions();

    private:
        bool createInstance(const char *applicationName);

        PhysicalDevice* selectDevice(WindowSurface &surface);

        bool isDeviceSuitable(const PhysicalDevice &device, WindowSurface &surface);

        uint32_t enumeratePhysicalDevices();

    private:
        std::vector<PhysicalDevice> m_physicalDevices;
        WindowSurface m_windowSurface;
        SwapChain m_swapChain;
        Device m_device;

        VkInstance m_handle;
    };

    inline VulkanContext::operator VkInstance() const {
        return m_handle;
    }

    inline Device& VulkanContext::getDevice() {
        return m_device;
    }

    inline const Device& VulkanContext::getDevice() const {
        return m_device;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_CONTEXT
