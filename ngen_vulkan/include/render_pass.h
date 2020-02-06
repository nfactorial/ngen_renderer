
#ifndef NGEN_VULKAN_RENDER_PASS
#define NGEN_VULKAN_RENDER_PASS

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;

    class RenderPass {
    public:
        RenderPass();
        ~RenderPass();

        void dispose();
        bool create(Device &device);

        operator VkRenderPass() const;  // NOLINT

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
