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

#ifndef NGEN_VULKAN_SWAP_CHAIN
#define NGEN_VULKAN_SWAP_CHAIN

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;
    class Device;

    class SwapChain {
    public:
        SwapChain();
        ~SwapChain();

        void dispose();

        void initialize(const PhysicalDevice &physicalDevice, const WindowSurface &surface);
        bool create(Device &device, WindowSurface &surface);

        operator VkSwapchainKHR () const; // NOLINT

        [[nodiscard]] bool isUsable() const;

        [[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat() const;
        [[nodiscard]] VkPresentModeKHR choosePresentMode() const;
        [[nodiscard]] VkExtent2D chooseExtent(int width, int height) const;

    private:
        void enumerateSurfaceFormats(const PhysicalDevice &physicalDevice, const WindowSurface &surface);
        void enumeratePresentModes(const PhysicalDevice &physicalDevice, const WindowSurface &surface);

        bool createImageViews();
        void extractImages();

        void destroyImageViews();

    private:
        std::vector<VkPresentModeKHR> m_presentModes;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkImageView> m_imageViews;
        std::vector<VkImage> m_images;

        VkSurfaceCapabilitiesKHR m_capabilities;

        VkSwapchainKHR m_handle;
        VkDevice m_device;
    };

    inline SwapChain::operator VkSwapchainKHR () const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_SWAP_CHAIN
