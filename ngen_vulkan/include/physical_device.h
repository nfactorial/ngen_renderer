
#ifndef NGEN_VULKAN_PHYSICAL_DEVICE
#define NGEN_VULKAN_PHYSICAL_DEVICE

////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "window_surface.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;

    class PhysicalDevice {
    public:
        PhysicalDevice();
        ~PhysicalDevice();

        void initialize(VkPhysicalDevice handle);

        [[nodiscard]] bool hasExtension(const char *extensionName) const;

        [[nodiscard]] int findQueueFamily(VkQueueFlags flags) const;
        [[nodiscard]] int findPresentationQueue(WindowSurface &surface) const;

        [[nodiscard]] VkPhysicalDevice getHandle() const;

        operator VkPhysicalDevice() const; // NOLINT

    private:
        void enumerateQueueFamilies();
        void enumerateExtensions();

    public:
        std::vector<VkQueueFamilyProperties> m_queueFamilies;
        std::vector<VkExtensionProperties> m_extensions;

        VkPhysicalDevice m_handle;
    };

    //! \brief Obtains the current vulkan physical device handle associated with this object.
    //! \returns The Vulkan physical device associated with the object.
    inline VkPhysicalDevice PhysicalDevice::getHandle() const {
        return m_handle;
    }

    //! \brief Operator overload to automatically convert the PhysicalDevice class to a VkPhysicalDevice object.
    inline PhysicalDevice::operator VkPhysicalDevice() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_PHYSICAL_DEVICE
