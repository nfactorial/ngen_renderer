
#include <cstdio>
#include <vector>
#include "device.h"
#include "physical_device.h"

namespace {
    // Spelled out so the provisional (beta) Vulkan headers aren't needed on every platform.
    const char *kPortabilitySubsetExtensionName = "VK_KHR_portability_subset";
}

namespace ngen::vulkan {
    Device::Device()
    : m_handle(VK_NULL_HANDLE)
    , m_presentationQueue(VK_NULL_HANDLE)
    , m_graphicsQueue(VK_NULL_HANDLE)
    , m_presentationQueueFamily(0)
    , m_graphicsQueueFamily(0) {

    }

    Device::~Device() {
        dispose();
    }

    //! \brief Destroys all resources belonging to this object.
    void Device::dispose() {
        waitIdle();

        if (VK_NULL_HANDLE != m_handle) {
            vkDestroyDevice(m_handle, nullptr);

            m_graphicsQueue = VK_NULL_HANDLE;
            m_presentationQueue = VK_NULL_HANDLE;
            m_handle = VK_NULL_HANDLE;
        }
    }

    //! \brief Waits for the device to become idle.
    void Device::waitIdle() const {
        if (VK_NULL_HANDLE != m_handle) {
            vkDeviceWaitIdle(m_handle);
        }
    }

    //! \brief Attempts to create an instance of the specified physical device.
    //! \param physicalDevice [in] - The physical device we are to create an instance of.
    //! \param surface [in] - The window surface to be used for rendering.
    //! \param extensionCount [in] - Number of extensions in the supplied extension list.
    //! \param requiredExtensions [in] - Pointer to list of extensions the application requires.
    //! \returns <em>True</em> if the device created successfully otherwise <em>false</em>.
    bool Device::create(PhysicalDevice &physicalDevice, WindowSurface &surface, size_t extensionCount, const char **requiredExtensions) {
        if (VK_NULL_HANDLE != m_handle) {
            return false;
        }

        const int graphicsFamily = physicalDevice.findQueueFamily(VK_QUEUE_GRAPHICS_BIT);
        const int presentationFamily = physicalDevice.findPresentationQueue(surface);
        if (graphicsFamily < 0 || presentationFamily < 0) {
            printf("Device is missing a graphics or presentation queue family.\n");
            return false;
        }

        const float queuePriority = 1.0f;

        // One queue per distinct family; requesting the same family twice is invalid.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (const int family : {graphicsFamily, presentationFamily}) {
            if (!queueCreateInfos.empty() && queueCreateInfos.front().queueFamilyIndex == static_cast<uint32_t>(family)) {
                continue;
            }

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(family);
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::vector<const char*> extensions(requiredExtensions, requiredExtensions + extensionCount);

        // The spec requires this extension to be enabled whenever a device exposes it (e.g. MoltenVK).
        if (physicalDevice.hasExtension(kPortabilitySubsetExtensionName)) {
            extensions.push_back(kPortabilitySubsetExtensionName);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_handle) != VK_SUCCESS) {
            return false;
        }

        m_graphicsQueueFamily = static_cast<uint32_t>(graphicsFamily);
        m_presentationQueueFamily = static_cast<uint32_t>(presentationFamily);

        vkGetDeviceQueue(m_handle, m_graphicsQueueFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_handle, m_presentationQueueFamily, 0, &m_presentationQueue);

        return true;
    }
}
