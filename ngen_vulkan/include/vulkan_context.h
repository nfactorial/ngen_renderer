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

#include <vulkan/vulkan.h>


////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        class VulkanContext {
        public:
            VulkanContext();
            ~VulkanContext();

            void dispose();
            bool initialize();

        private:
            bool createInstance();

            VkPhysicalDevice selectDevice();
            VkDevice createDevice(VkPhysicalDevice physicalDevice);

            bool isDeviceSuitable(VkPhysicalDevice device);
            int findQueueFamily(VkPhysicalDevice device, VkQueueFlags flags) const;

        private:
            VkQueue m_graphicsQueue;
            VkInstance m_instance;
            VkDevice m_device;
        };
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_CONTEXT
