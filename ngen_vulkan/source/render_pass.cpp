
#include "render_pass.h"
#include "command_pool.h"
#include "device.h"

namespace ngen::vulkan {
    const VkRenderPass RenderPass::kInvalidHandle = 0l;

    RenderPass::RenderPass()
    : m_commandBuffer(VK_NULL_HANDLE)
    , m_handle(kInvalidHandle)
    , m_device(nullptr) {
    }

    RenderPass::~RenderPass() {
        dispose();
    }

    //! Destroys any resources allocated by this object.
    void RenderPass::dispose() {
        if (m_handle != kInvalidHandle) {
            vkDestroyRenderPass(m_device, m_handle, nullptr);

            m_handle = kInvalidHandle;
            m_device = nullptr;
        }
    }

    //! \brief  Prepares the object for use by the application.
    //! \param  device [in] - The device to use when creating the render pass.
    //! \returns <em>True</em> if the render pass was created successfully otherwise <em>false</em>.
    bool RenderPass::create(Device &device) {
        // TODO: Need to be able to specify sub-passes and color attachments
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

    //! \brief Begins the command recording using a command buffer contained within the supplied command pool.
    //! \param commandPool [in] - The command pool that contains the command buffer to be used.
    //! \param bufferIndex [in] - Index of the buffer to be used within the command pool.
    //! \returns <em>True</em> if the render pass began successfully otherwise <em>false</em>.
    bool RenderPass::begin(const CommandPool &commandPool, size_t bufferIndex) {
        if (m_handle == kInvalidHandle || m_commandBuffer != VK_NULL_HANDLE) {
            return false;   // Not initialized or already started
        }

        m_commandBuffer = commandPool.begin(bufferIndex);
        if (m_commandBuffer == VK_NULL_HANDLE) {
            return false;   // Failed to start
        }

        // TODO: Clears, swap-chains, graphics pipelines etc. need to be configurable
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_handle;
        // renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        // renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        return true;x
    }

    //! \brief Informs the render pass that the recording of render commands has completed.
    void RenderPass::end() {
        if (m_commandBuffer != VK_NULL_HANDLE) {
            vkCmdEndRenderPass(m_commandBuffer);
            vkEndCommandBuffer(m_commandBuffer);

            m_commandBuffer = VK_NULL_HANDLE;
        }

        // TODO: Need to be able to execute it
    }
}
