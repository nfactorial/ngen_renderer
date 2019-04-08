
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
    }

    //! \brief Attempts to create an instance of the specified physical device.
    //! \param physicalDevice [in] - The physical device we are attempting to create an instance of.
    //! \returns The created device or VK_NULL_HANDLE if one could not be created.
    bool PhysicalDevice::createDevice(Device &device, WindowSurface &surface) const {
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = findQueueFamily(VK_QUEUE_GRAPHICS_BIT);
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;

        // TODO: Device should be passed in, for us to fill out
        VkDevice deviceHandle;
        if (vkCreateDevice(m_handle, &createInfo, nullptr, &deviceHandle) == VK_SUCCESS) {
            return device.initialize(deviceHandle, windowSurface, this);
        }

        return false;
    }

    //! \brief Extracts the list of queue families available for this physical device.
    void PhysicalDevice::enumerateQueueFamilies() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, nullptr);

        m_queueFamilies.clear();

        if (queueFamilyCount != 0) {
            m_queueFamilies.resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, m_queueFamilies.data());
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

            vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, index, surface.getSurface(), &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                return index;
            }

            index++;
        }

        return -1;
    }
}
