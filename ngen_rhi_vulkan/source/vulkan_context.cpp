// Spec: rhi-vulkan. Behaviour changes go through it.
#include "vulkan_context.h"

#include "device_selection.h"
#include "surface.h"
#include "vulkan_result.h"

#include <cstring>
#include <utility>
#include <vector>

namespace ngen::rhi::vulkan
{
    namespace
    {
        // Spelled out so the provisional (beta) Vulkan headers are not needed.
        constexpr const char *kPortabilitySubsetExtension = "VK_KHR_portability_subset";

        bool hasLayer(const char *const name)
        {
            uint32_t count = 0;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> layers(count);
            vkEnumerateInstanceLayerProperties(&count, layers.data());
            for (const VkLayerProperties &layer : layers)
            {
                if (std::strcmp(layer.layerName, name) == 0)
                {
                    return true;
                }
            }
            return false;
        }

        // The extensions the loader (layer == nullptr) or a layer offers.
        std::vector<VkExtensionProperties> instanceExtensions(const char *const layer)
        {
            uint32_t count = 0;
            vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(layer, &count, extensions.data());
            extensions.resize(count);
            return extensions;
        }

        bool contains(const std::vector<VkExtensionProperties> &extensions, const char *const name)
        {
            for (const VkExtensionProperties &extension : extensions)
            {
                if (std::strcmp(extension.extensionName, name) == 0)
                {
                    return true;
                }
            }
            return false;
        }

        std::unexpected<ngen::Error> unsupported(std::string message)
        {
            return std::unexpected(makeError(ErrorCode::Unsupported, std::move(message)));
        }

        // Only warnings and errors are requested, so each message is reported as one or the other.
        VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *const data,
                                                     void *const userData)
        {
            const auto *context = static_cast<const VulkanContext *>(userData);
            const Severity level =
                (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0 ? Severity::Error : Severity::Warning;
            context->report(level, data->pMessage != nullptr ? data->pMessage : "(no message text)");
            return VK_FALSE;
        }

        VkDebugUtilsMessengerCreateInfoEXT messengerInfo(const VulkanContext *const context)
        {
            VkDebugUtilsMessengerCreateInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            info.pfnUserCallback = debugCallback;
            info.pUserData = const_cast<VulkanContext *>(context);
            return info;
        }
    } // namespace

    std::expected<std::unique_ptr<VulkanContext>, ngen::Error> VulkanContext::create(
        const std::string &applicationName, const bool enableValidation, const bool windowExpected, MessageSink sink,
        const char *const validationLayer)
    {
        // Validation is checked before anything is created, so a missing layer is reported as such.
        if (enableValidation && !hasLayer(validationLayer))
        {
            return unsupported(std::string("validation was requested but the ") + validationLayer +
                               " layer is missing (install the Vulkan SDK, or create the device without validation)");
        }

        std::unique_ptr<VulkanContext> context(new VulkanContext(std::move(sink), windowExpected));
        if (auto created = context->createInstance(applicationName, enableValidation, validationLayer); !created)
        {
            return std::unexpected(std::move(created.error()));
        }
        if (auto selected = context->selectPhysicalDevice(); !selected)
        {
            return std::unexpected(std::move(selected.error()));
        }
        if (auto created = context->createDevice(); !created)
        {
            return std::unexpected(std::move(created.error()));
        }
        if (auto created = context->createAllocator(); !created)
        {
            return std::unexpected(std::move(created.error()));
        }
        return context;
    }

    VulkanContext::VulkanContext(MessageSink sink, const bool windowExpected)
        : sink_(std::move(sink)), windowExpected_(windowExpected)
    {
        if (!sink_)
        {
            sink_ = makeDefaultMessageSink();
        }
    }

    VulkanContext::~VulkanContext()
    {
        if (allocator_ != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(allocator_);
        }
        if (device_ != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device_, nullptr);
        }
        if (messenger_ != VK_NULL_HANDLE)
        {
            const auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));
            if (destroyMessenger != nullptr)
            {
                destroyMessenger(instance_, messenger_, nullptr);
            }
        }
        if (instance_ != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    void VulkanContext::report(const Severity severity, const std::string_view message) const
    {
        sink_(severity, message);
    }

    std::expected<void, ngen::Error> VulkanContext::createInstance(const std::string &applicationName,
                                                                   const bool enableValidation,
                                                                   const char *const validationLayer)
    {
        const std::vector<VkExtensionProperties> available = instanceExtensions(nullptr);
        std::vector<const char *> extensions;

        if (windowExpected_)
        {
            const char *const surfaceExtension = platformSurfaceExtension();
            if (surfaceExtension == nullptr)
            {
                return unsupported("this platform has no Vulkan surface extension, so no window can be attached");
            }
            for (const char *const name : {VK_KHR_SURFACE_EXTENSION_NAME, surfaceExtension})
            {
                if (!contains(available, name))
                {
                    return unsupported(std::string("the Vulkan loader does not offer the instance extension ") + name);
                }
                extensions.push_back(name);
            }
        }

        // Portability drivers (MoltenVK) are only enumerated when the application opts in.
        VkInstanceCreateFlags flags = 0;
        if (contains(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }

        std::vector<const char *> layers;
        if (enableValidation)
        {
            if (!contains(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) &&
                !contains(instanceExtensions(validationLayer), VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                return unsupported("validation was requested but VK_EXT_debug_utils is not offered");
            }
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            layers.push_back(validationLayer);
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "ngen";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        // Chaining the messenger's description also reports problems in instance creation and destruction.
        const VkDebugUtilsMessengerCreateInfoEXT debugInfo = messengerInfo(this);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = enableValidation ? &debugInfo : nullptr;
        createInfo.flags = flags;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_); result != VK_SUCCESS)
        {
            instance_ = VK_NULL_HANDLE;
            return std::unexpected(vulkanError("vkCreateInstance", result));
        }

        if (enableValidation)
        {
            const auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT"));
            if (createMessenger == nullptr)
            {
                return std::unexpected(
                    makeError(ErrorCode::Unsupported, "vkCreateDebugUtilsMessengerEXT is not available"));
            }
            if (const VkResult result = createMessenger(instance_, &debugInfo, nullptr, &messenger_);
                result != VK_SUCCESS)
            {
                messenger_ = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateDebugUtilsMessengerEXT", result));
            }
        }
        return {};
    }

    std::expected<void, ngen::Error> VulkanContext::selectPhysicalDevice()
    {
        uint32_t count = 0;
        if (const VkResult result = vkEnumeratePhysicalDevices(instance_, &count, nullptr); result != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkEnumeratePhysicalDevices", result));
        }
        std::vector<VkPhysicalDevice> devices(count);
        if (const VkResult result = vkEnumeratePhysicalDevices(instance_, &count, devices.data());
            result != VK_SUCCESS && result != VK_INCOMPLETE)
        {
            return std::unexpected(vulkanError("vkEnumeratePhysicalDevices", result));
        }
        devices.resize(count);

        std::vector<PhysicalDeviceCandidate> candidates;
        std::vector<bool> portabilitySubset;
        for (const VkPhysicalDevice device : devices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);

            PhysicalDeviceCandidate candidate;
            candidate.name = properties.deviceName;
            candidate.discrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            candidate.apiVersion = properties.apiVersion;

            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
            for (uint32_t family = 0; family < familyCount; ++family)
            {
                if (families[family].queueCount == 0 || (families[family].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
                {
                    continue;
                }
                if (!candidate.graphicsFamily.has_value())
                {
                    candidate.graphicsFamily = family;
                }
                if (windowExpected_ && !candidate.presentFamily.has_value() && platformCanPresent(device, family))
                {
                    candidate.presentFamily = family;
                }
            }

            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
            extensions.resize(extensionCount);
            candidate.hasSwapchain = contains(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            portabilitySubset.push_back(contains(extensions, kPortabilitySubsetExtension));

            candidates.push_back(std::move(candidate));
        }

        const auto choice = choosePhysicalDevice(candidates, windowExpected_);
        if (!choice)
        {
            return unsupported(choice.error());
        }

        physicalDevice_ = devices[choice->index];
        hasPortabilitySubset_ = portabilitySubset[choice->index];
        queueFamily_ = choice->queueFamily;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
        limits_ = properties.limits;
        return {};
    }

    std::expected<void, ngen::Error> VulkanContext::createDevice()
    {
        const float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily_;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;

        std::vector<const char *> extensions;
        if (windowExpected_)
        {
            extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }
        // The portability specification requires this extension whenever a device offers it.
        if (hasPortabilitySubset_)
        {
            extensions.push_back(kPortabilitySubsetExtension);
        }

        // PolygonMode::Line needs fillModeNonSolid; it is enabled wherever the device offers it.
        VkPhysicalDeviceFeatures supported{};
        vkGetPhysicalDeviceFeatures(physicalDevice_, &supported);
        VkPhysicalDeviceFeatures features{};
        features.fillModeNonSolid = supported.fillModeNonSolid;
        fillModeNonSolid_ = supported.fillModeNonSolid == VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pEnabledFeatures = &features;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (const VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_);
            result != VK_SUCCESS)
        {
            device_ = VK_NULL_HANDLE;
            return std::unexpected(vulkanError("vkCreateDevice", result));
        }
        vkGetDeviceQueue(device_, queueFamily_, 0, &queue_);
        return {};
    }

    std::expected<void, ngen::Error> VulkanContext::createAllocator()
    {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.physicalDevice = physicalDevice_;
        createInfo.device = device_;
        createInfo.instance = instance_;
        createInfo.vulkanApiVersion = VK_API_VERSION_1_1;

        if (const VkResult result = vmaCreateAllocator(&createInfo, &allocator_); result != VK_SUCCESS)
        {
            allocator_ = VK_NULL_HANDLE;
            return std::unexpected(vulkanError("vmaCreateAllocator", result));
        }
        return {};
    }
} // namespace ngen::rhi::vulkan
