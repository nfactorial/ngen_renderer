
#ifndef NGEN_VULKAN_RENDER_PASS
#define NGEN_VULKAN_RENDER_PASS

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;
    class Pipeline;
    class CommandPool;
    class VulkanContext;

    class RenderPass {
    public:
        RenderPass();
        ~RenderPass();

        RenderPass(const RenderPass &other) = delete;
        RenderPass& operator=(const RenderPass &other) = delete;

        void dispose();
        [[nodiscard]] bool create(const VulkanContext &context);

        [[nodiscard]] bool begin(const VulkanContext &context, const CommandPool &commandPool, VkFramebuffer frameBuffer, size_t bufferIndex);
        void end();

        operator VkRenderPass() const;  // NOLINT

        static const VkRenderPass kInvalidHandle;

        void bind(const Pipeline &pipeline);
        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

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
