
#include "swap_chain.h"
#include "physical_device.h"

namespace ngen::vulkan {
    SwapChain::SwapChain() {
    }

    SwapChain::~SwapChain() {
        dispose();
    }

    void SwapChain::dispose() {

    }

    //! \brief Extracts the SwapChain information from the supplied device and surface.
    //! \param physicalDevice [in] - The PhysicalDevice we are enumerating surface formats for.
    //! \param surface [in] - The surface the device will be rendering to.
    void SwapChain::initialize(const PhysicalDevice &physicalDevice, const WindowSurface &surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.getHandle(), surface.getSurface(), &m_capabilities);

        enumerateSurfaceFormats(physicalDevice, surface);
        enumeratePresentModes(physicalDevice, surface);
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
    VkExtent2D SwapChain::chooseExtent(uint32_t width, uint32_t height) const {
        if (m_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return m_capabilities.currentExtent;
        }

        VkExtent2D extent = {width, height};

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
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getSurface(), &formatCount, nullptr);

        m_formats.clear();
        if (formatCount) {
            m_formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getSurface(), &formatCount, m_formats.data());
        }
    }

    //! \brief Determines which present modes are supported by the supplied device with the specified surface.
    //! \param physicalDevice [in] - The PhysicalDevice we are enumerating surface formats for.
    //! \param surface [in] - The surface the device will be rendering to.
    void SwapChain::enumeratePresentModes(const PhysicalDevice &physicalDevice, const WindowSurface &surface) {
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getSurface(), &presentModeCount, nullptr);

        m_presentModes.clear();
        if (presentModeCount) {
            m_presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getSurface(), &presentModeCount, m_presentModes.data());
        }
    }
}
