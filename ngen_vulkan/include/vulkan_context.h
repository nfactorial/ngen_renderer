//
// Copyright 2017 nfactorial
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

#ifndef NGEN_VULKAN_CONTEXT
#define NGEN_VULKAN_CONTEXT

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "physical_device.h"
#include "window_surface.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class WindowSurface;

    class VulkanContext {
    public:
        VulkanContext();
        ~VulkanContext();

        void dispose();
        bool initialize(HWND hwnd);

    private:
        bool createInstance();

        VkPhysicalDevice selectDevice();
        VkDevice createDevice(const PhysicalDevice &physicalDevice);

        bool isDeviceSuitable(const PhysicalDevice &device);
        int findQueueFamily(const PhysicalDevice &device, VkQueueFlags flags) const;
        int findPresentationQueue(const PhysicalDevice &device, WindowSurface &surface) const;

    private:
        std::vector<PhysicalDevice> m_physicalDevices;

        WindowSurface m_windowSurface;
        VkInstance m_instance;
        VkDevice m_device;

        VkQueue m_presentationQueue;
        VkQueue m_graphicsQueue;
    };
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_CONTEXT
