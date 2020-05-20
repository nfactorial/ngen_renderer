
#include "buffer.h"
#include "vulkan_context.h"

namespace ngen::vulkan {
    Buffer::Buffer()
    : m_size(0)
    , m_memory(VK_NULL_HANDLE)
    , m_handle(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {

    }

    Buffer::~Buffer() {
        dispose();
    }

    //! \brief Releases all resources currently associated with this object.
    void Buffer::dispose() {
        if (m_device != VK_NULL_HANDLE) {
            if (m_handle != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_device, m_handle, nullptr);
                m_handle = VK_NULL_HANDLE;
            }

            if (m_memory != VK_NULL_HANDLE) {
                vkFreeMemory(m_device, m_memory, nullptr);
                m_memory = VK_NULL_HANDLE;
            }

            m_device = VK_NULL_HANDLE;
            m_size = 0;
        }
    }

    //! \brief Attempts to create a vertex buffer.
    //! \param context [in] - The Vulkan context we will be associated with.
    //! \param length [in] - The length (in bytes) of the buffer.
    //! \returns <em>True</em> if the buffer created successfully otherwise <em>false</em>.
    bool Buffer::createVertexBuffer(const VulkanContext &context, size_t length) {
        return create(context, length, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }

    //! \brief Creates a buffer object suitable for holding uniform data.
    //! \param context [in] - The vulkan context we will be associated with.
    //! \param length [in] - The length (in bytes) of the buffer.
    //! \returns <em>True</em> if the buffer created successfully otherwise <em>false</em>.
    bool Buffer::createUniformBuffer(const VulkanContext &context, size_t length) {
        return create(context, length, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }

    //! \brief Creates a new buffer using the supplied description.
    //! \param context [in] - The Vulkan context we will be associated with.
    //! \param length [in] - The length (in bytes) of the buffer.
    //! \param usage [in] - Describes how the buffer will be used within the application.
    //! \param sharingMode [in] - How the buffer is shared within the application.
    //! \returns <em>True</em> if the buffer created successfully otherwise <em>false</em>.
    bool Buffer::create(const VulkanContext &context, size_t length, VkBufferUsageFlags usage, VkSharingMode sharingMode) {
        if (m_device != VK_NULL_HANDLE) {
            return false;
        }

        VkBufferCreateInfo bufferInfo{};

        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = length;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = sharingMode;

        if (VK_SUCCESS != vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &m_handle)) {
            return false;
        }

        m_device = context.getDevice();

        if (!allocate(context)) {
            dispose();
            return false;
        }

        m_size = length;
        vkBindBufferMemory(context.getDevice(), m_handle, m_memory, 0);

        return true;
    }

    //! \brief Binds the buffer to the supplied command buffer.
    //! \param commandBuffer [in] - The command buffer to bind with.
    void Buffer::bind(VkCommandBuffer commandBuffer) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_handle, &offset);
    }

    //! \brief Attempts to map the entire buffer memory for access by the application.
    //! \remarks Once you have finished with the mapped memory, you must call Buffer::unmapMemory().
    //! \param context [in] - The Vulkan context we will be associated with.
    //! \returns Pointer to the mapped memory or <em>nullptr</em> if it could not be accessed.
    void* Buffer::mapMemory() {
        return mapMemory(0, m_size);
    }

    //! \brief Attempts to map a subset of the buffer memory for use by the application.
    //! \remarks Once you have finished with the mapped memory, you must call Buffer::unmapMemory().
    //! \param context [in] - The Vulkan context we will be associated with.
    //! \param offset [in] - Offset within the buffer memory the mapping will begin.
    //! \param length [in] - The number of bytes to be mapped.
    //! \returns Pointer to the mapped memory or <em>nullptr</em> if it could not be accessed.
    void* Buffer::mapMemory(size_t offset, size_t length) {
        void *data = nullptr;

        if (m_handle != VK_NULL_HANDLE) {
            vkMapMemory(m_device, m_memory, offset, length, 0, &data);
        }

        return data;
    }

    void Buffer::unmapMemory() {
        vkUnmapMemory(m_device, m_memory);
    }

    //! \brief Attempts to allocate memory for use with the buffer object.
    //! \param context [in] - The Vulkan context we will be associated with.
    //! \returns <em>True</em> if the memory allocated successfully otherwise <em>false</em>.
    bool Buffer::allocate(const VulkanContext &context) {
        return allocate(context, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    //! \brief Attempts to allocate memory for use with the buffer object using the supplied memory properties.
    //! \param context [in] - The Vulkan context we will be associated with.
    //! \param properties [in] - Properties of the memory block to be allocated.
    //! \returns <em>True</em> if the memory allocated successfully otherwise <em>false</em>.
    bool Buffer::allocate(const VulkanContext &context, const uint32_t properties) {
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(context.getDevice(), m_handle, &memoryRequirements);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(*context.getPhysicalDevice(), &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if (memoryRequirements.memoryTypeBits & (1u << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                VkMemoryAllocateInfo allocInfo{};

                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memoryRequirements.size;
                allocInfo.memoryTypeIndex = i;

                return (VK_SUCCESS == vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &m_memory));
            }
        }

        return false;
    }
}
