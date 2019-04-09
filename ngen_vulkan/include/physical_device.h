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

#ifndef NGEN_VULKAN_PHYSICAL_DEVICE
#define NGEN_VULKAN_PHYSICAL_DEVICE

////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "window_surface.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;

    class PhysicalDevice {
    public:
        PhysicalDevice();
        ~PhysicalDevice();

        void initialize(VkPhysicalDevice handle);

        bool hasExtension(const char *extensionName) const;

        int findQueueFamily(VkQueueFlags flags) const;
        int findPresentationQueue(WindowSurface &surface) const;

        VkPhysicalDevice getHandle() const;

    private:
        void enumerateQueueFamilies();
        void enumerateExtensions();

    public:
        std::vector<VkQueueFamilyProperties> m_queueFamilies;
        std::vector<VkExtensionProperties> m_extensions;

        VkPhysicalDevice m_handle;
    };

    //! \brief Obtains the current vulkan physical device handle associated with this object.
    //! \returns The Vulkan physical device associated with the object.
    inline VkPhysicalDevice PhysicalDevice::getHandle() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_PHYSICAL_DEVICE
