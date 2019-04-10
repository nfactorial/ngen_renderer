//
// Copyright 2019 nfactorial
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
