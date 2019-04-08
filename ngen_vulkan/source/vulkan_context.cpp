//
// Created by nfact on 02/04/2019.
//

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdio>
#include "vulkan_context.h"
#include "vulkan_error.h"

namespace {
    const char *kEngineName = "nGen";
    const uint32_t kEngineVersion = VK_MAKE_VERSION(1, 0, 0);

    const uint32_t kVulkanExtensionCount = 2;
    const char *kVulkanExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface"
    };
}

namespace ngen::vulkan {
    VulkanContext::VulkanContext()
    : m_instance(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE)
    , m_presentationQueue(VK_NULL_HANDLE)
    , m_graphicsQueue(VK_NULL_HANDLE) {
    }

    VulkanContext::~VulkanContext() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void VulkanContext::dispose() {
        if (VK_NULL_HANDLE != m_instance) {
            m_graphicsQueue = VK_NULL_HANDLE;
            m_presentationQueue = VK_NULL_HANDLE;

            if (m_device != VK_NULL_HANDLE) {
                vkDestroyDevice(m_device, nullptr);
                m_device = VK_NULL_HANDLE;
            }

            m_windowSurface.dispose(m_instance);

            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
        }
    }

    //! \brief Prepares the object for use by the application.
    //! \param hwnd [in] - The handle of the applications main window.
    //! \returns True if the object initialized successfully otherwise false.
    bool VulkanContext::initialize(HWND hwnd, const char *applicationName) { // TODO: Should be platform agnostic
        if (m_device != VK_NULL_HANDLE) {
            printf("VulkanContext already initialized\n");
            return false;
        }

        if (createInstance(applicationName)) {
            uint32_t extensionCount;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            printf("Listing extensions:\n");
            for (const auto &extension : extensions) {
                printf("\t%s\n", extension.extensionName);
            }

            if (m_windowSurface.initialize(m_instance, hwnd)) {
                PhysicalDevice *physicalDevice = selectDevice();
                if (physicalDevice) {
                    m_device = physicalDevice->createDevice(m_windowSurface);
                    if (VK_NULL_HANDLE != m_device) {
                        return true;
                    }
                } else {
                    printf("Unable to find suitable device for rendering.\n");
                }
            }
        }

        dispose();
        return false;
    }

    //! \brief Creates the Vulkan instance for use by the application.
    //! \param applicationName [in] - The name associated with the running application.
    //! \returns True if the instance was created successfully otherwise false.
    bool VulkanContext::createInstance(const char *applicationName) {
        VkApplicationInfo appInfo = {};

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = kEngineName;
        appInfo.engineVersion = kEngineVersion;
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = kVulkanExtensionCount;
        createInfo.ppEnabledExtensionNames = kVulkanExtensions;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
        if (result != VK_SUCCESS) {
            printf("Failed to create instance: %s!\n", getResultString(result));
            return false;
        }

        printf("Successfully created Vulkan instance!\n");
        return true;
    }

    //! \brief Enumerates the available devices and selects one for use.
    //! \returns The physical device that was selected for use by the application.
    PhysicalDevice* VulkanContext::selectDevice() {
        enumeratePhysicalDevices();

        for (auto &device : m_physicalDevices) {
            if (isDeviceSuitable(device)) {
                printf("Selected device for rendering\n");
                return &device;
            }
        }

        return nullptr;
    }

    //! \brief Determines whether or not the specified physical device is suitable for the application to use.
    //! \param physicalDevice [in] - The physical device we are checking for compatability.
    //! \returns True if the device is suitable otherwise false.
    bool VulkanContext::isDeviceSuitable(const PhysicalDevice &physicalDevice, WindowSurface &surface) {
        const int graphicsQueue = physicalDevice.findQueueFamily(VK_QUEUE_GRAPHICS_BIT);
        const int presentationQueue = physicalDevice.findPresentationQueue(surface);

        if (-1 == graphicsQueue) {
            printf("Unable to find graphics queue\n");
            return false;
        }

        if (-1 == presentationQueue) {
            printf("Unable to find presentation queue\n");
            return false;
        }

        return true;
    }

    //! \brief Obtains a list of physical devices supported by the host machine
    //! \returns The number of physical devices found by the method.
    uint32_t VulkanContext::enumeratePhysicalDevices() {
        uint32_t deviceCount = 0;

        if (m_instance != VK_NULL_HANDLE) {
            m_physicalDevices.clear();

            vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

            m_physicalDevices.resize(deviceCount);
            if (deviceCount != 0) {
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

                for (uint32_t index = 0; index < deviceCount; ++index) {
                    m_physicalDevices[index].initialize(devices[index]);
                }
            }
        }

        return deviceCount;
    }
}