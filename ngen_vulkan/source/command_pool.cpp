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

    //! \brief Releases all resources currently referenced by this object.
    void CommandPool::dispose() {
        if (m_device != VK_NULL_HANDLE && m_handle != VK_NULL_HANDLE) {
            if (m_commandBuffers.size() != 0) {
                vkFreeCommandBuffers(m_device, m_handle, m_commandBuffers.size(), m_commandBuffers.data());
                m_commandBuffers.clear();
            }

            vkDestroyCommandPool(m_device, m_handle, nullptr);

            m_device = VK_NULL_HANDLE;
            m_handle = VK_NULL_HANDLE;
        }
    }

    //! \brief Prepares the command pool for use with the specified device and queue.
    bool CommandPool::create(const Device &device, const PhysicalDevice &physicalDevice, VkQueueFlags queueFlags) {
        return create(device, physicalDevice, queueFlags, 0);
    }

    //! \brief Prepares the command pool for use with the specified device and queue along with specified creation flags.
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

    //! \brief Attempts to allocate the specified number of command buffers within the pool.
    //! \remarks If there are buffer already created, they will be discarded.
    //! \param count [in] - The number of command buffers to be allocated.
    //! \returns True if the specified number of command buffers created successfully otherwise false.
    bool CommandPool::allocateCommandBuffers(size_t count) {
        if (m_commandBuffers.size() != 0) {
            vkFreeCommandBuffers(m_device, m_handle, m_commandBuffers.size(), m_commandBuffers.data());
            m_commandBuffers.clear();
        }

        m_commandBuffers.resize(count);
        printf("m_commandBuffers.size is %d\n", (int)m_commandBuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};

        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
        allocInfo.commandPool = m_handle;

        if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            m_commandBuffers.clear();
            return false;
        }

        return false;
    }

    //! \brief Begins the recording of a specified command buffer within the pool.
    //! \remarks It is left to the caller to invoke vkEndCommandBuffer once they have finished with the command buffer.
    //! \returns The handle to the command pool or VK_NULL_HANDLE if recording failed to begin.
    VkCommandBuffer CommandPool::begin(size_t index) const {
        if (index < m_commandBuffers.size()) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(m_commandBuffers[index], &beginInfo) == VK_SUCCESS) {
                return m_commandBuffers[index];
            }
        }

        return VK_NULL_HANDLE;
    }
}
