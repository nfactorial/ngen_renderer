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

////////////////////////////////////////////////////////////////////////////

#include "device.h"
#include "physical_device.h"


namespace ngen::vulkan {
    Device::Device()
    : m_handle(VK_NULL_HANDLE) {

    }

    Device::~Device() {

    }

    //! \brief Prepares the object instance for use by the application.
    //! \param handle [in] - Handle of the Vulkan device instance we are representing.
    //! \param surface [in] - The window surface we will be presenting to.
    //! \param physicalDevice [in] - The PhysicalDevice object we came from.
    //! \returns True if the Device object initialized successfully otherwise false.
    bool Device::initialize(VkDevice handle, WindowSurface &surface, PhysicalDevice *physicalDevice) {
        m_handle = handle;

        vkGetDeviceQueue(handle, physicalDevice->findQueueFamily(VK_QUEUE_GRAPHICS_BIT), 0, &m_graphicsQueue);
        vkGetDeviceQueue(handle, physicalDevice->findPresentationQueue(surface), 0, &m_presentationQueue);

        return true;
    }
}
