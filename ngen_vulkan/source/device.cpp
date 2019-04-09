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

////////////////////////////////////////////////////////////////////////////

#include "device.h"
#include "physical_device.h"


namespace ngen::vulkan {
    Device::Device()
    : m_handle(VK_NULL_HANDLE)
    , m_presentationQueue(VK_NULL_HANDLE)
    , m_graphicsQueue(VK_NULL_HANDLE) {

    }

    Device::~Device() {

    }

    //! \brief Destroys all resources belonging to this object.
    void Device::dispose() {
        // TODO: Delete handle
    }

    //! \brief Attempts to create an instance of the specified physical device.
    //! \param physicalDevice [in] - The physical device we are to create an instance of.
    //! \param surface [in] - The window surface to be used for rendering.
    //! \returns The created device or VK_NULL_HANDLE if one could not be created.
    bool Device::create(PhysicalDevice &physicalDevice, WindowSurface &surface) {
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = physicalDevice.findQueueFamily(VK_QUEUE_GRAPHICS_BIT);
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;

        // TODO: Device should be passed in, for us to fill out
        if (vkCreateDevice(physicalDevice.getHandle(), &createInfo, nullptr, &m_handle) == VK_SUCCESS) {
            vkGetDeviceQueue(m_handle, physicalDevice.findQueueFamily(VK_QUEUE_GRAPHICS_BIT), 0, &m_graphicsQueue);
            vkGetDeviceQueue(m_handle, physicalDevice.findPresentationQueue(surface), 0, &m_presentationQueue);
            return true;
        }

        return false;
    }
}
