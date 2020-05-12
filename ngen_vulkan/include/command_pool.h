
#ifndef NGEN_VULKAN_COMMAND_POOL
#define NGEN_VULKAN_COMMAND_POOL

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;
    class PhysicalDevice;

    class CommandPool {
    public:
        CommandPool();
        ~CommandPool();

        CommandPool(const CommandPool &other) = delete;
        CommandPool& operator=(const CommandPool &other) = delete;

        void dispose();
        bool create(const Device &device, const PhysicalDevice &physicalDevice, VkQueueFlags queueFlags);
        bool create(const Device &device, const PhysicalDevice &physicalDevice, VkQueueFlags queueFlags, int flags);

        bool allocateCommandBuffers(size_t count);

        operator VkCommandPool() const;  // NOLINT

    private:
        std::vector<VkCommandBuffer> m_commandBuffers;

        VkDevice m_device;
        VkCommandPool m_handle;
    };

    inline CommandPool::operator VkCommandPool() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_COMMAND_POOL
