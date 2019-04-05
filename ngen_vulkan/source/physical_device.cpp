
#include "physical_device.h"

namespace ngen::vulkan {
    PhysicalDevice::PhysicalDevice()
    : m_handle(VK_NULL_HANDLE) {
    }

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice)
    : m_handle(physicalDevice) {
    }

    PhysicalDevice::~PhysicalDevice() {
    }

    //! \brief Retrieves a list of Vulkan Physical devices available on the supplied instance.
    //! \param result [in] - Container for the enuemrated devices.
    //! \param instance [in] - The vulkan instance to be used for enumeration.
    uint32_t PhysicalDevice::enumerate(std::vector<PhysicalDevice> &result, VkInstance instance) {
        uint32_t deviceCount = 0;

        if (instance != VK_NULL_HANDLE) {
            result.clear();

            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount != 0) {
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

                for (const auto &device : devices) {
                    result.push_back(PhysicalDevice(device));
                }
            }
        }

        return deviceCount;
    }

    //! \brief Extracts the list of queue families available for this physical device.
    void PhysicalDevice::extractQueueFamilies() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, nullptr);

        m_queueFamilies.clear();

        if (queueFamilyCount != 0) {
            m_queueFamilies.resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, m_queueFamilies.data());
        }
    }
}
