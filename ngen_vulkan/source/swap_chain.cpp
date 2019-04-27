
#include "swap_chain.h"
#include "physical_device.h"
#include "device.h"
#include "vulkan_error.h"

namespace ngen::vulkan {
    SwapChain::SwapChain()
    : m_handle(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {
    }

    SwapChain::~SwapChain() {
        dispose();
    }

    void SwapChain::dispose() {
        if (VK_NULL_HANDLE != m_handle) {
            vkDestroySwapchainKHR(m_device, m_handle, nullptr);
            m_handle = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }

    }

    //! \brief Extracts the SwapChain information from the supplied device and surface.
    //! \param physicalDevice [in] - The PhysicalDevice we are enumerating surface formats for.
    //! \param surface [in] - The surface the device will be rendering to.
    void SwapChain::initialize(const PhysicalDevice &physicalDevice, const WindowSurface &surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &m_capabilities);

        enumerateSurfaceFormats(physicalDevice, surface);
        enumeratePresentModes(physicalDevice, surface);
    }

    //! \brief Creates a swap chain suitable for use with the application
    //! \param device [in] - The device that will be used for rendering.
    //! \param surface [in] - The WindowSurface we will be presenting to.
    //! \param width [in] - The width of the display (in pixels).
    //! \param height [in] - The height of the display (in pixels).
    //! \returns True if the swap chain created successfully otherwise false.
    bool SwapChain::create(Device &device, WindowSurface &surface) {
        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat();

        uint32_t imageCount = m_capabilities.minImageCount + 1;
        if (m_capabilities.maxImageCount > 0 && imageCount > m_capabilities.maxImageCount) {
            imageCount = m_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = chooseExtent(surface.getWidth(), surface.getHeight());
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = m_capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = choosePresentMode();
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (device.getGraphicsQueue() == device.getPresentationQueue()) {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            //createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_handle);
        if (result != VK_SUCCESS) {
            printf("Failed to create SwapChain: %s\n", getResultString(result));
            return false;
        }

        m_device = device;
        return true;
    }

    //! \brief Determines whether or not the SwapChain object is suitable for rendering.
    //! \returns True if the swap chain is usable otherwise false.
    bool SwapChain::isUsable() const {
        return !m_formats.empty() && !m_presentModes.empty();
    }

    //! \brief Determines the swap extents to be used with the swap chain.
    //! \param width [in] - The width of the target surface (in pixels).
    //! \param height [in] - The height of the target surface (in pixels).
    //! \returns The swap extents to be used with the swap chain.
    VkExtent2D SwapChain::chooseExtent(int width, int height) const {
        if (m_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return m_capabilities.currentExtent;
        }

        VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        extent.width = std::max(m_capabilities.minImageExtent.width, std::min(m_capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(m_capabilities.minImageExtent.height, std::min(m_capabilities.maxImageExtent.height, extent.height));

        return extent;
    }

    //! \brief Selects a surface format suitable for use by the renderer.
    VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat() const {
        if (m_formats.size() == 1 && m_formats[0].format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        for (const auto &availableFormat : m_formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return m_formats[0];
    }

    //! \brief Determines which presentation mode should be used when rendering.
    //! \returns The presentation mode the application should use when presenting the display.
    VkPresentModeKHR SwapChain::choosePresentMode() const {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto &presentMode : m_presentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }

            if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = presentMode;
            }
        }

        return bestMode;
    }

    //! \brief Determines which surface formats the supplied device supports that are compatible with the surface.
    //! \param physicalDevice [in] - The PhysicalDevice we are enumerating surface formats for.
    //! \param surface [in] - The surface the device will be rendering to.
    void SwapChain::enumerateSurfaceFormats(const PhysicalDevice &physicalDevice, const WindowSurface &surface) {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

        m_formats.clear();
        if (formatCount) {
            m_formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, m_formats.data());
        }
    }

    //! \brief Determines which present modes are supported by the supplied device with the specified surface.
    //! \param physicalDevice [in] - The PhysicalDevice we are enumerating surface formats for.
    //! \param surface [in] - The surface the device will be rendering to.
    void SwapChain::enumeratePresentModes(const PhysicalDevice &physicalDevice, const WindowSurface &surface) {
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        m_presentModes.clear();
        if (presentModeCount) {
            m_presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, m_presentModes.data());
        }
    }
}
