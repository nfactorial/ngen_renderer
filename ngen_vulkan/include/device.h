
#ifndef NGEN_VULKAN_DEVICE
#define NGEN_VULKAN_DEVICE

////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "platform.h"
#include "swap_chain.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;

    class Device {
    public:
        Device();
        ~Device();

        bool create(PhysicalDevice &physicalDevice, WindowSurface &surface, size_t extensionCount, const char **requiredExtensions);
        void dispose();

        [[nodiscard]] VkQueue getPresentationQueue() const;
        [[nodiscard]] VkQueue getGraphicsQueue() const;

        operator VkDevice() const; // NOLINT

    public:
        SwapChain m_swapChain;
        VkDevice m_handle;

        VkQueue m_presentationQueue;
        VkQueue m_graphicsQueue;
    };

    //! \brief Retrieves the queue suitable for presentation.
    //! \returns The presentation queue to be used for the device.
    inline VkQueue Device::getPresentationQueue() const {
        return m_presentationQueue;
    }

    //! \brief Retrieves the queue suitable for graphics rendering.
    //! \returns The graphics queue to be used for the device.
    inline VkQueue Device::getGraphicsQueue() const {
        return m_graphicsQueue;
    }

    //! \brief Operator overload to convert a Device object to a raw VkDevice.
    inline Device::operator VkDevice() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_DEVICE
