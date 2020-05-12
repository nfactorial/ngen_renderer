
#ifndef NGEN_VULKAN_COMMAND_BUFFER
#define NGEN_VULKAN_COMMAND_BUFFER

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class CommandBuffer {
    public:
        CommandBuffer();
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer &other) = delete;
        CommandBuffer& operator=(const CommandBuffer &other) = delete;

        void dispose();
        bool create(CommandPool &pool, size_t count);

        void end();
        bool begin(size_t index);

        operator VkCommandBuffer() const;  // NOLINT

    private:
        VkCommandBuffer m_handle;
    };

    inline CommandBuffer::operator VkCommandBuffer() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_COMMAND_BUFFER
