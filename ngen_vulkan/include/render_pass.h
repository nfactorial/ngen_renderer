
#ifndef NGEN_VULKAN_RENDER_PASS
#define NGEN_VULKAN_RENDER_PASS

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;
    class CommandPool;

    class RenderPass {
    public:
        RenderPass();
        ~RenderPass();

        RenderPass(const RenderPass &other) = delete;
        RenderPass& operator=(const RenderPass &other) = delete;

        void dispose();
        [[nodiscard]] bool create(Device &device);

        [[nodiscard]] bool begin(const CommandPool &commandPool, size_t bufferIndex);
        void end();

        operator VkRenderPass() const;  // NOLINT

        static const VkRenderPass kInvalidHandle;

    private:
        VkCommandBuffer m_commandBuffer;
        VkRenderPass m_handle;
        VkDevice m_device;
    };

    inline RenderPass::operator VkRenderPass() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_RENDER_PASS
