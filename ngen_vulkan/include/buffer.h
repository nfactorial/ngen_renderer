
#ifndef NGEN_VULKAN_BUFFER
#define NGEN_VULKAN_BUFFER

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class VulkanContext;

    class Buffer {
    public:
        Buffer();
        ~Buffer();

        Buffer(const Buffer &other) = delete;
        Buffer& operator=(const Buffer &other) = delete;

        void dispose();

        [[nodiscard]] bool createVertexBuffer(const VulkanContext &context, size_t length);

        void bind(VkCommandBuffer commandBuffer);

        [[nodiscard]] void* mapMemory();
        [[nodiscard]] void* mapMemory(size_t offset, size_t length);
        void unmapMemory();

        operator VkBuffer() const;  // NOLINT

    private:
        bool create(const VulkanContext &context, size_t length, VkBufferUsageFlags usage, VkSharingMode sharingMode);
        bool allocate(const VulkanContext &context);

    private:
        size_t m_size;
        VkDeviceMemory m_memory;
        VkBuffer m_handle;
        VkDevice m_device;
    };

    inline Buffer::operator VkBuffer() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_BUFFER
