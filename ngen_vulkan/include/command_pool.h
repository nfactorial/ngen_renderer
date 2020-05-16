
#ifndef NGEN_VULKAN_COMMAND_POOL
#define NGEN_VULKAN_COMMAND_POOL

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "platform.h"
#include "vulkan_context.h"


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
        [[nodiscard]] bool create(const VulkanContext &context, VkQueueFlags queueFlags);
        [[nodiscard]] bool create(const VulkanContext &context, VkQueueFlags queueFlags, int flags);

        [[nodiscard]] bool allocateCommandBuffers(size_t count);

        [[nodiscard]] VkCommandBuffer begin(size_t index) const;
        void end(size_t index) const;

        [[nodiscard]] size_t size() const;

        const VkCommandBuffer& operator[](size_t index) const;

        operator VkCommandPool() const;  // NOLINT

    private:
        std::vector<VkCommandBuffer> m_commandBuffers;

        VkDevice m_device;
        VkCommandPool m_handle;
    };

    //! \brief Retrieves the number of command buffer contained within the pool.
    //! \returns The number of command buffers contained within the pool.
    inline size_t CommandPool::size() const {
        return m_commandBuffers.size();
    }

    inline const VkCommandBuffer& CommandPool::operator[](size_t index) const {
        return m_commandBuffers[index];
    }

    inline CommandPool::operator VkCommandPool() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_COMMAND_POOL
