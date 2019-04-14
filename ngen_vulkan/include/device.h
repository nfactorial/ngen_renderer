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

#ifndef NGEN_VULKAN_DEVICE
#define NGEN_VULKAN_DEVICE

////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;

    class Device {
    public:
        Device();
        ~Device();

        bool create(PhysicalDevice &physicalDevice, WindowSurface &surface, size_t extensionCount, const char **requiredExtensions);
        void dispose();

        VkQueue getPresentationQueue() const;
        VkQueue getGraphicsQueue() const;

        operator VkDevice() const; // NOLINT

    public:
        VkDevice m_handle;

        VkQueue m_presentationQueue;
        VkQueue m_graphicsQueue;
    };

    //! \brief Retrieves the queue suitable for presentation.
    //! \returns The presentation queue to be used for the device.
    inline VkQueue Device::getPresentationQueue() const {
        return m_presentationQueue;
    }

    //! \brief Retrieves the queue suitable for graphics rendering.
    //! \returns The graphics queue to be used for the device.
    inline VkQueue Device::getGraphicsQueue() const {
        return m_graphicsQueue;
    }

    //! \brief Operator overload to convert a Device object to a raw VkDevice.
    inline Device::operator VkDevice() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_DEVICE
