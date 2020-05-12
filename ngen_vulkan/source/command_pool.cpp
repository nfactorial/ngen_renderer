#include "command_pool.h"
#include "physical_device.h"
#include "vulkan_error.h"
#include "device.h"

namespace ngen::vulkan {
    CommandPool::CommandPool()
    : m_device(VK_NULL_HANDLE)
    , m_handle(VK_NULL_HANDLE) {
    }

    CommandPool::~CommandPool() {
        dispose();
    }

    void CommandPool::dispose() {
        if (m_device != VK_NULL_HANDLE && m_handle != VK_NULL_HANDLE) {
            m_commandBuffers.clear();

            vkDestroyCommandPool(m_device, m_handle, nullptr);

            m_device = VK_NULL_HANDLE;
            m_handle = VK_NULL_HANDLE;
        }
    }

    bool CommandPool::create(const Device &device, const PhysicalDevice &physicalDevice, VkQueueFlags queueFlags) {
        return create(device, physicalDevice, queueFlags, 0);
    }

    bool CommandPool::create(const Device &device, const PhysicalDevice &physicalDevice, VkQueueFlags queueFlags, int flags) {
        VkCommandPoolCreateInfo poolInfo{};

        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = physicalDevice.findQueueFamily(queueFlags);
        poolInfo.flags = flags;

        const auto result = vkCreateCommandPool(device, &poolInfo, nullptr, &m_handle);
        if ( result != VK_SUCCESS) {
            printf("Failed to create command pool: %s.\n", getResultString(result));
            return false;
        }

        return true;
    }

    bool CommandPool::allocateCommandBuffers(size_t count) {
        m_commandBuffers.clear();

        m_commandBuffers.resize(count);
        printf("m_commandBuffers.size is %d\n", (int)m_commandBuffers.size());

        return false;
    }
}
