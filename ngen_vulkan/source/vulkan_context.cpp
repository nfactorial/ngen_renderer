#include <vector>
#include <cstdio>
#include <cstring>
#include "vulkan_context.h"
#include "vulkan_error.h"

namespace {
    const char *kEngineName = "nGen";
    const uint32_t kEngineVersion = VK_MAKE_VERSION(1, 0, 0);
    // Vulkan 1.1 is the Android baseline, and the oldest version Slang's SPIR-V output (1.3) runs on.
    const uint32_t kVulkanSdkVersion = VK_API_VERSION_1_1;

    const char* kValidationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
}

namespace {
    bool hasInstanceExtension(const char *name) {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

        for (const auto &extension : extensions) {
            if (0 == strcmp(extension.extensionName, name)) {
                return true;
            }
        }

        return false;
    }

    //! \brief Reports validation messages; only warnings and errors are requested, so anything here is a problem.
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                 VkDebugUtilsMessageTypeFlagsEXT,
                                                 const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                 void *) {
        const char *label = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "error" : "warning";
        fprintf(stderr, "Vulkan %s: %s\n", label, data->pMessage);
        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT makeMessengerInfo() {
        VkDebugUtilsMessengerCreateInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        info.pfnUserCallback = debugCallback;
        return info;
    }
}

namespace ngen::vulkan {
    VulkanContext::VulkanContext()
    : m_handle(VK_NULL_HANDLE)
    , m_debugMessenger(VK_NULL_HANDLE)
    , m_physicalDevice(nullptr) {
    }

    VulkanContext::~VulkanContext() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void VulkanContext::dispose() {
        if (VK_NULL_HANDLE != m_handle) {
            m_swapChain.dispose();
            m_device.dispose();
            m_windowSurface.dispose();

            if (VK_NULL_HANDLE != m_debugMessenger) {
                auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(m_handle, "vkDestroyDebugUtilsMessengerEXT"));
                if (destroyMessenger) {
                    destroyMessenger(m_handle, m_debugMessenger, nullptr);
                }
                m_debugMessenger = VK_NULL_HANDLE;
            }

            vkDestroyInstance(m_handle, nullptr);

            m_physicalDevice = nullptr;
            m_handle = VK_NULL_HANDLE;
        }
    }

    //! \brief Debug method, outputs a list of supported extensions to the standard output.
    void VulkanContext::dumpExtensions() {
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        printf("Listing extensions:\n");
        for (const auto &extension : extensions) {
            printf("\t%s\n", extension.extensionName);
        }
    }

    //! \brief Prepares the object for use by the application.
    //! \param window [in] - The handle of the applications main window.
    //! \param applicationName [in] - The name of the application.
    //! \param enableValidation [in] - If <em>True</em> vulkan will be created with validation layers.
    //! \returns True if the object initialized successfully otherwise false.
    bool VulkanContext::initialize(SDL_Window *window, const char *applicationName, bool enableValidation) {
        if (m_handle != VK_NULL_HANDLE) {
            printf("VulkanContext already initialized\n");
            return false;
        }

        if (create(window, applicationName, enableValidation)) {
            if (m_windowSurface.initialize(m_handle, window)) {
                m_physicalDevice = selectDevice(m_windowSurface);
                if (m_physicalDevice && m_device.create(*m_physicalDevice, m_windowSurface, platform::kDefaultDeviceExtensionCount, platform::kDefaultDeviceExtensions)) {
                    m_swapChain.initialize(*m_physicalDevice, m_windowSurface);
                    if (m_swapChain.create(m_device, m_windowSurface)) {
                        return true;
                    }

                    m_physicalDevice = nullptr;

                    printf("Failed to create swap chain.");
                } else {
                    printf("Unable to find suitable device for rendering.\n");
                }
            }
        }

        dispose();
        return false;
    }

    //! \brief Event handler invoked by the application when the window has changed its dimensions.
    //! \returns <em>True</em> if the context handled the resize successfully otherwise <em>false</em>.
    bool VulkanContext::onWindowResized() {
        return m_windowSurface.onWindowResized();
    }

    //! \brief Creates the Vulkan instance for use by the application.
    //! \param applicationName [in] - The name associated with the running application.
    //! \param enableValidation [in] - If <em>True</em> vulkan will be created with validation layers.
    //! \returns True if the instance was created successfully otherwise false.
    bool VulkanContext::create(SDL_Window *window, const char *applicationName, bool enableValidation) {
        if (enableValidation) {
            if (!checkValidationLayerSupport()) {
                printf("Validation requested but VK_LAYER_KHRONOS_validation is not installed.\n");
                return false;
            }

            return createInstance(window, applicationName, sizeof(kValidationLayers) / sizeof(kValidationLayers[0]), kValidationLayers);
        }

        return createInstance(window, applicationName, 0, nullptr);
    }

    //! \brief Determines whether or not the specified validation layers are supported.
    //! \returns <em>True</em> if the validation layers are supported otherwise <em>false</em>.
    bool VulkanContext::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : kValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    //! \brief Attempts to create the vulkan instance with the supplied information.
    //! \param window [in] - The window we will present to; SDL reports the instance extensions it needs.
    //! \param applicationName [in] - The name associated with the running application.
    //! \param layerCount [in] - The number of layers specified in the layerNames parameters.
    //! \param layerNames [in] - List of layers that should be enabled on the created device.
    //! \returns True if the instance was created successfully otherwise false.
    bool VulkanContext::createInstance(SDL_Window *window, const char *applicationName, uint32_t layerCount, const char* const* layerNames) {
        unsigned int sdlExtensionCount = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr)) {
            printf("Failed to query Vulkan instance extensions: %s\n", SDL_GetError());
            return false;
        }

        std::vector<const char*> extensions(sdlExtensionCount);
        SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, extensions.data());

        // Portability drivers (e.g. MoltenVK) are only enumerated when the application opts in.
        VkInstanceCreateFlags flags = 0;
        if (hasInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }

        // With validation on, route its messages through our callback; chaining the info also covers instance creation.
        const VkDebugUtilsMessengerCreateInfoEXT messengerInfo = makeMessengerInfo();
        if (layerCount > 0) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        VkApplicationInfo appInfo = {};

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = kEngineName;
        appInfo.engineVersion = kEngineVersion;
        appInfo.apiVersion = kVulkanSdkVersion;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.flags = flags;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.ppEnabledLayerNames = layerNames;
        createInfo.enabledLayerCount = layerCount;
        createInfo.pNext = layerCount > 0 ? &messengerInfo : nullptr;

        const auto result = vkCreateInstance(&createInfo, nullptr, &m_handle);
        if (result != VK_SUCCESS) {
            printf("Failed to create instance: %s!\n", getResultString(result));
            return false;
        }

        if (layerCount > 0) {
            auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_handle, "vkCreateDebugUtilsMessengerEXT"));
            if (!createMessenger || VK_SUCCESS != createMessenger(m_handle, &messengerInfo, nullptr, &m_debugMessenger)) {
                printf("Failed to create debug messenger, validation messages will not be reported.\n");
            }
        }

        return true;
    }

    //! \brief Enumerates the available devices and selects one for use.
    //! \param surface [in] - The window surface to be used for rendering.
    //! \returns The physical device that was selected for use by the application.
    PhysicalDevice* VulkanContext::selectDevice(WindowSurface &surface) {
        enumeratePhysicalDevices();

        for (auto &device : m_physicalDevices) {
            if (device.isDeviceSuitable(surface)) {
                return &device;
            }
        }

        return nullptr;
    }

    //! \brief Obtains a list of physical devices supported by the host machine
    //! \returns The number of physical devices found by the method.
    uint32_t VulkanContext::enumeratePhysicalDevices() {
        uint32_t deviceCount = 0;

        if (m_handle != VK_NULL_HANDLE) {
            m_physicalDevices.clear();

            vkEnumeratePhysicalDevices(m_handle, &deviceCount, nullptr);

            m_physicalDevices.resize(deviceCount);
            if (deviceCount != 0) {
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(m_handle, &deviceCount, devices.data());

                for (uint32_t index = 0; index < deviceCount; ++index) {
                    m_physicalDevices[index].initialize(devices[index]);
                }
            }
        }

        return deviceCount;
    }
}
