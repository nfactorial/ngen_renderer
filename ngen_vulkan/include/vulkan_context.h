
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

    //! \brief Maintains core information about the Vulkan rendering system.
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
        [[nodiscard]] PhysicalDevice* getPhysicalDevice();
        [[nodiscard]] const PhysicalDevice* getPhysicalDevice() const;

        [[nodiscard]] SwapChain& getSwapChain();
        [[nodiscard]] const SwapChain& getSwapChain() const;

        static void dumpExtensions();

    private:
        bool createInstance(const char *applicationName);

        PhysicalDevice* selectDevice(WindowSurface &surface);

        uint32_t enumeratePhysicalDevices();

    private:
        std::vector<PhysicalDevice> m_physicalDevices;
        PhysicalDevice *m_physicalDevice;
        WindowSurface m_windowSurface;
        SwapChain m_swapChain;
        Device m_device;

        VkInstance m_handle;
    };

    inline VulkanContext::operator VkInstance() const {
        return m_handle;
    }

    //! \brief Retrieves the active device associated with the context.
    //! \returns The device currently associated with the context.
    inline Device& VulkanContext::getDevice() {
        return m_device;
    }

    //! \brief Retrieves the active device associated with the context.
    //! \returns The device currently associated with the context.
    inline const Device& VulkanContext::getDevice() const {
        return m_device;
    }

    //! \brief Retrieves the active physical device associated with the context.
    //! \returns The physical device currently associated with the context or nullptr if there is none active.
    inline PhysicalDevice* VulkanContext::getPhysicalDevice() {
        return m_physicalDevice;
    }

    //! \brief Retrieves the active physical device associated with the context.
    //! \returns The physical device currently associated with the context or nullptr if there is none active.
    inline const PhysicalDevice* VulkanContext::getPhysicalDevice() const {
        return m_physicalDevice;
    }

    //! \brief Retrieves the swap chain associated with the context.
    //! \returns The swap chain currently associated with the context.
    inline SwapChain& VulkanContext::getSwapChain() {
        return m_swapChain;
    }

    //! \brief Retrieves the swap chain associated with the context.
    //! \returns The swap chain currently associated with the context.
    inline const SwapChain& VulkanContext::getSwapChain() const {
        return m_swapChain;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_CONTEXT
