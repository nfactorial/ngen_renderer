
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

        [[nodiscard]] bool begin(const VkExtent2D &extent, VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer) const;
        void end(VkCommandBuffer commandBuffer);

        operator VkRenderPass() const;  // NOLINT

        static const VkRenderPass kInvalidHandle;

    private:
        VkRenderPass m_handle;
        VkDevice m_device;
    };

    inline RenderPass::operator VkRenderPass() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_RENDER_PASS
