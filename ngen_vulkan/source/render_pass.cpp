
#include "render_pass.h"
#include "device.h"

namespace ngen::vulkan {
    RenderPass::RenderPass()
    : m_handle(nullptr)
    , m_device(nullptr) {
    }

    RenderPass::~RenderPass() {
        dispose();
    }

    //! Destroys any resources allocated by this object.
    void RenderPass::dispose() {
        if (m_handle != nullptr) {
            vkDestroyRenderPass(m_device, m_handle, nullptr);

            m_handle = nullptr;
            m_device = nullptr;
        }
    }

    //! \brief  Prepares the object for use by the application.
    //! \param  device [in] - The device to use when creating the render pass.
    //! \returns <em>True</em> if the render pass was created successfully otherwise <em>false</em>.
    bool RenderPass::create(Device &device) {
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = nullptr;//&colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = nullptr;//&subpass;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS) {
            printf("Failed to create render pass");
            return false;
        }

        return false;
    }
}
