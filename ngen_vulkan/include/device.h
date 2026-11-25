
#ifndef NGEN_VULKAN_DEVICE
#define NGEN_VULKAN_DEVICE

////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;

    class Device {
    public:
        Device();
        ~Device();

        Device(const Device &other) = delete;
        Device& operator=(const Device &other) = delete;

        bool create(PhysicalDevice &physicalDevice, WindowSurface &surface, size_t extensionCount, const char **requiredExtensions);
        void dispose();

        void waitIdle() const;

        [[nodiscard]] VkQueue getPresentationQueue() const;
        [[nodiscard]] VkQueue getGraphicsQueue() const;

        operator VkDevice() const; // NOLINT

        [[nodiscard]] uint32_t getPresentationQueueFamily() const;
        [[nodiscard]] uint32_t getGraphicsQueueFamily() const;

    private:
        VkDevice m_handle;

        VkQueue m_presentationQueue;
        VkQueue m_graphicsQueue;

        uint32_t m_presentationQueueFamily;
        uint32_t m_graphicsQueueFamily;
    };

    //! \brief Retrieves the index of the queue family used for presentation.
    inline uint32_t Device::getPresentationQueueFamily() const {
        return m_presentationQueueFamily;
    }

    //! \brief Retrieves the index of the queue family used for graphics rendering.
    inline uint32_t Device::getGraphicsQueueFamily() const {
        return m_graphicsQueueFamily;
    }

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
