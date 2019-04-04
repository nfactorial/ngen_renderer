//
// Created by nfact on 02/04/2019.
//

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdio>
#include "vulkan_context.h"
#include "vulkan_error.h"

namespace {
    const uint32_t kVulkanExtensionCount = 2;
    const char *kVulkanExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface"
    };
}

namespace ngen {
    namespace rendering {
        VulkanContext::VulkanContext()
        : m_instance(VK_NULL_HANDLE)
        , m_device(VK_NULL_HANDLE)
        , m_presentationQueue(VK_NULL_HANDLE)
        , m_graphicsQueue(VK_NULL_HANDLE) {
        }

        VulkanContext::~VulkanContext() {
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
        //! \returns True if the object initialized successfully otherwise false.
        bool VulkanContext::initialize(HWND hwnd) { // TODO: Should be platform agnostic
            if (m_device != VK_NULL_HANDLE) {
                printf("VulkanContext already initialized\n");
                return false;
            }

            if (createInstance()) {
                uint32_t extensionCount;
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

                printf("Listing extensions:\n");
                for (const auto &extension : extensions) {
                    printf("\t%s\n", extension.extensionName);
                }

                if (m_windowSurface.initialize(m_instance, hwnd)) {
                    m_device = createDevice(selectDevice());
                    if (VK_NULL_HANDLE != m_device) {
                        return true;
                    }
                }
            }

            dispose();
            return false;
        }

        bool VulkanContext::createInstance() {
            VkApplicationInfo appInfo = {};

            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "nGen Vulkan Example";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "nGen";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = kVulkanExtensionCount;
            createInfo.ppEnabledExtensionNames = kVulkanExtensions;

            VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
            if (result != VK_SUCCESS) {
                printf("Failed to create instance: %s!\n", getVulkanResultString(result));
                return false;
            }

            printf("Successfully created Vulkan instance!\n");
            return true;
        }

        //! \brief Enumerates the available devices and selects one for use.
        //! \returns The physical device that was selected for use by the application.
        VkPhysicalDevice VulkanContext::selectDevice() {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

            if (deviceCount != 0) {
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

                for (const auto &device : devices) {
                    if (isDeviceSuitable(device)) {
                        printf("Selected device for rendering\n");
                        return device;
                    }
                }

                printf("Unable to find a suitable device for use by the application.\n");
            } else {
                printf("Cannot find GPU with Vulkan support\n");
            }

            return VK_NULL_HANDLE;
        }

        //! \brief Attempts to create an instance of the specified physical device.
        //! \param physicalDevice [in] - The physical device we are attempting to create an instance of.
        //! \returns The created device or VK_NULL_HANDLE if one could not be created.
        VkDevice VulkanContext::createDevice(VkPhysicalDevice physicalDevice) {
            if (physicalDevice != VK_NULL_HANDLE) {
                float queuePriority = 1.0f;

                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = findQueueFamily(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
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

                VkDevice device;
                if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS) {
                    vkGetDeviceQueue(device, findQueueFamily(physicalDevice, VK_QUEUE_GRAPHICS_BIT), 0, &m_graphicsQueue);
                    vkGetDeviceQueue(device, findPresentationQueue(physicalDevice, m_windowSurface), 0, &m_presentationQueue);
                    return device;
                }
            }

            return VK_NULL_HANDLE;
        }

        //! \brief Determines whether or not the specified physical device is suitable for the application to use.
        //! \param physicalDevice [in] - The physical device we are checking for compatability.
        //! \returns True if the device is suitable otherwise false.
        bool VulkanContext::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
            const int graphicsQueue = findQueueFamily(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
            const int presentationQueue = findPresentationQueue(physicalDevice, m_windowSurface);

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

        //! \brief Retrieves the index of the specified queue within the specified physical device.
        //! \param physicalDevice [in] - The device whose available queues are to be checked.
        //! \param flags [in] - The flags of the particular queue we are to scan for.
        //! \returns Index of the requested queue within the device or -1 if it could not be found.
        int VulkanContext::findQueueFamily(VkPhysicalDevice device, VkQueueFlags flags) const {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int index = 0;
            for (const auto &queueFamily : queueFamilies) {
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
        int VulkanContext::findPresentationQueue(VkPhysicalDevice device, WindowSurface &surface) const {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int index = 0;
            for (const auto &queueFamily : queueFamilies) {
                VkBool32 presentSupport = false;

                vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface.getSurface(), &presentSupport);
                if (queueFamily.queueCount > 0 && presentSupport) {
                    return index;
                }

                index++;
            }

            printf("Scanned %u queues but found no presentation support\n", queueFamilyCount);
            return -1;
        }
    }
}