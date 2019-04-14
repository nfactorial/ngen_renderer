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

#ifndef NGEN_VULKAN_CONTEXT
#define NGEN_VULKAN_CONTEXT

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "physical_device.h"
#include "window_surface.h"
#include "device.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;

    class VulkanContext {
    public:
        VulkanContext();
        ~VulkanContext();

        void dispose();
        bool initialize(PlatformWindow platformWindow, const char *applicationName);

        operator VkInstance() const; // NOLINT

    private:
        bool createInstance(const char *applicationName);

        PhysicalDevice* selectDevice(WindowSurface &surface);

        bool isDeviceSuitable(const PhysicalDevice &device, WindowSurface &surface);

        uint32_t enumeratePhysicalDevices();

    private:
        std::vector<PhysicalDevice> m_physicalDevices;
        WindowSurface m_windowSurface;
        Device m_device;

        VkInstance m_instance;
    };

    inline VulkanContext::operator VkInstance() const {
        return m_instance;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_CONTEXT
