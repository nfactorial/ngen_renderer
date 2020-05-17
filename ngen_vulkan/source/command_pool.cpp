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
            if (!m_commandBuffers.empty()) {
                vkFreeCommandBuffers(m_device, m_handle, m_commandBuffers.size(), m_commandBuffers.data());
                m_commandBuffers.clear();
            }

            vkDestroyCommandPool(m_device, m_handle, nullptr);

            m_device = VK_NULL_HANDLE;
            m_handle = VK_NULL_HANDLE;
        }
    }

    //! \brief Prepares the command pool for use with the specified device and queue.
    //! \param context [in] - Properties of the Vulkan instance we will be interacting with.
    //! \param queueFlags [in] - The queue our command pool will be associated with.
    //! \returns <em>True</em> if the command pool created successfully otherwise <em>false</em>.
    bool CommandPool::create(const VulkanContext &context, VkQueueFlags queueFlags) {
        return create(context, queueFlags, 0);
    }

    //! \brief Prepares the command pool for use with the specified device and queue along with specified creation flags.
    //! \param context [in] - Properties of the Vulkan instance we will be interacting with.
    //! \param queueFlags [in] - The queue our command pool will be associated with.
    //! \param flags [in] - Bit mask of VkCommandPoolCreateFlagBits indicating usage behaviour.
    //! \returns <em>True</em> if the command pool created successfully otherwise <em>false</em>.
    bool CommandPool::create(const VulkanContext &context, VkQueueFlags queueFlags, int flags) {
        VkCommandPoolCreateInfo poolInfo{};

        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = context.getPhysicalDevice()->findQueueFamily(queueFlags);
        poolInfo.flags = flags;

        printf("Command queue for queueFamilyIndex %d\n", poolInfo.queueFamilyIndex);

        const auto result = vkCreateCommandPool(context.getDevice(), &poolInfo, nullptr, &m_handle);
        if (result != VK_SUCCESS) {
            printf("Failed to create command pool: %s.\n", getResultString(result));
            return false;
        }

        m_device = context.getDevice();
        return true;
    }

    //! \brief Attempts to allocate the specified number of command buffers within the pool.
    //! \remarks If there are buffer already created, they will be discarded.
    //! \param count [in] - The number of command buffers to be allocated.
    //! \returns True if the specified number of command buffers created successfully otherwise false.
    bool CommandPool::allocateCommandBuffers(size_t count) {
        if (m_device == VK_NULL_HANDLE) {
            return false;
        }

        printf("Attempting to create %d command buffers...\n", int(count));
        if (!m_commandBuffers.empty()) {
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

        return true;
    }

    //! \brief Begins the recording of a specified command buffer within the pool.
    //! \param index [in] - Index of the command buffer to be used.
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

    //! \brief Signals the end of recording for a specified buffer within the pool.
    //! \param index [in] - Index of the command buffer whose command recording is being ended.
    void CommandPool::end(size_t index) const {
        vkEndCommandBuffer(m_commandBuffers[index]);
    }
}
