
#include "render_pass.h"
#include "vulkan_context.h"
#include "device.h"

namespace ngen::vulkan {
    const VkRenderPass RenderPass::kInvalidHandle = 0l;

    RenderPass::RenderPass()
    : m_handle(kInvalidHandle)
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
    //! \param  context [in] - The vulkan context we will be interacting with.
    //! \returns <em>True</em> if the render pass was created successfully otherwise <em>false</em>.
    bool RenderPass::create(const VulkanContext &context) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = context.getSwapChain().getImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency {};

        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        // TODO: Need to be able to specify sub-passes and color attachments
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(context.getDevice(), &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS) {
            printf("Failed to create render pass");
            return false;
        }

        m_device = context.getDevice();
        return true;
    }

    //! \brief Begins the command recording using a command buffer contained within the supplied command pool.
    //! \param extent [in] - The rendering extents to be used for the render pass.
    //! \param commandBuffer [in] - The command buffer we will be rendering to.
    //! \param frameBuffer [in] - The frame buffer we will be rendering to.
    //! \returns <em>True</em> if the render pass began successfully otherwise <em>false</em>.
    bool RenderPass::begin(const VkExtent2D &extent, VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer) const {
        if (m_handle == kInvalidHandle || commandBuffer == VK_NULL_HANDLE || frameBuffer == VK_NULL_HANDLE) {
            return false;
        }

        // TODO: Clears, swap-chains, graphics pipelines etc. need to be configurable
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_handle;
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        return true;
    }

    //! \brief Informs the command buffer that recording with the render pass has completed.
    //! \param commandBuffer [in] - The command buffer that has finished recording with ther render pass.
    void RenderPass::end(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }
}
