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

#ifndef NGEN_VULKAN_PHYSICAL_DEVICE
#define NGEN_VULKAN_PHYSICAL_DEVICE

////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "window_surface.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice {
    public:
        PhysicalDevice();
        explicit PhysicalDevice(VkPhysicalDevice physicalDevice);
        ~PhysicalDevice();

        static uint32_t enumerate(std::vector<PhysicalDevice> &result, VkInstance instance);

        operator VkPhysicalDevice() const;

    private:
        void extractQueueFamilies();

    private:
        std::vector<VkQueueFamilyProperties> m_queueFamilies;

        VkPhysicalDevice m_handle;
    };

    inline PhysicalDevice::operator VkPhysicalDevice() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_PHYSICAL_DEVICE
