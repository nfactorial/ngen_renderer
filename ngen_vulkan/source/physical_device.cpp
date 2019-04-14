
#include <string>
#include "physical_device.h"
#include "window_surface.h"
#include "device.h"

namespace ngen::vulkan {
    PhysicalDevice::PhysicalDevice()
    : m_handle(VK_NULL_HANDLE) {
    }

    PhysicalDevice::~PhysicalDevice() {
    }

    //! \brief Prepares the object for use by the application.
    //! \param handle [in] - The Vulkan physical device handle this object will represent.
    void PhysicalDevice::initialize(VkPhysicalDevice handle) {
        m_handle = handle;

        enumerateQueueFamilies();
        enumerateExtensions();
    }

    //! \brief Determines whether or not the physical device supports the specified extension.
    //! \param extensionName [in] - The name of the Vulkan extension to be checked.
    //! \returns True if the specified extension is supported by the physical device otherwise false.
    bool PhysicalDevice::hasExtension(const char *extensionName) const {
        for (const auto &extension : m_extensions) {
            if (0 == strcmp(extension.extensionName, extensionName)) {
                return true;
            }
        }

        return false;
    }

    //! \brief Extracts the list of queue families available for this physical device.
    void PhysicalDevice::enumerateQueueFamilies() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, nullptr);

        m_queueFamilies.clear();

        if (queueFamilyCount) {
            m_queueFamilies.resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, m_queueFamilies.data());
        }
    }

    //! \brief Retrieves a list of supported extensions for this device.
    void PhysicalDevice::enumerateExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extensionCount, nullptr);

        m_extensions.clear();
        if (extensionCount) {
            m_extensions.resize(extensionCount);
            vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extensionCount, m_extensions.data());
        }
    }

    //! \brief Scans the queue families of the physical device for one that matches the requested flags.
    //! \param flags [in] - Flags to search for within the queue families.
    //! \returns Index of the requested queue or -1 if it could not be found.
    int PhysicalDevice::findQueueFamily(VkQueueFlags flags) const {
        int index = 0;
        for (const auto &queueFamily : m_queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & flags) {
                return index;
            }

            index++;
        }

        return -1;
    }

    //! \brief Retrieves the index of the specified queue within the specified physical device.
    //! \param physicalDevice [in] - The device whose available queues are to be checked.
    //! \param flags [in] - The flags of the particular queue we are to scan for.
    //! \returns Index of the requested queue within the device or -1 if it could not be found.
    int PhysicalDevice::findPresentationQueue(WindowSurface &surface) const {
        int index = 0;
        for (const auto &queueFamily : m_queueFamilies) {
            VkBool32 presentSupport = false;

            vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, index, surface, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                return index;
            }

            index++;
        }

        return -1;
    }
}
