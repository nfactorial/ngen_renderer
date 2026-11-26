// Spec: rhi-vulkan. Behaviour changes go through it.
#include "render_pass_cache.h"

#include "vulkan_result.h"

namespace ngen::rhi::vulkan
{
    namespace
    {
        VkAttachmentLoadOp toVulkan(const LoadOp load)
        {
            switch (load)
            {
            case LoadOp::Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case LoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case LoadOp::DontCare:
                break;
            }
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }

        VkAttachmentStoreOp toVulkan(const StoreOp store)
        {
            return store == StoreOp::Store ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    } // namespace

    RenderPassCache::RenderPassCache(const VkDevice device, const VkImageLayout targetLayout)
        : device_(device), targetLayout_(targetLayout)
    {
    }

    RenderPassCache::~RenderPassCache()
    {
        for (auto entry = entries_.rbegin(); entry != entries_.rend(); ++entry)
        {
            vkDestroyRenderPass(device_, entry->renderPass, nullptr);
        }
    }

    std::expected<VkRenderPass, ngen::Error> RenderPassCache::get(const VkFormat format, const LoadOp load,
                                                                  const StoreOp store)
    {
        for (const Entry &entry : entries_)
        {
            if (entry.format == format && entry.load == load && entry.store == store)
            {
                return entry.renderPass;
            }
        }

        // Loading keeps what an earlier pass left in the image, which is then in the target layout; every other
        // load op discards the contents, so the image may start in any layout.
        VkAttachmentDescription attachment{};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = toVulkan(load);
        attachment.storeOp = toVulkan(store);
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = load == LoadOp::Load ? targetLayout_ : VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = targetLayout_;

        VkAttachmentReference reference{};
        reference.attachment = 0;
        reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &reference;

        // Orders the attachment's load and layout transition after the acquire semaphore's wait (at the colour
        // output stage) and after any earlier pass's writes to the same image in this frame.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &attachment;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        if (const VkResult result = vkCreateRenderPass(device_, &createInfo, nullptr, &renderPass);
            result != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkCreateRenderPass", result));
        }
        entries_.push_back(Entry{format, load, store, renderPass});
        return renderPass;
    }
} // namespace ngen::rhi::vulkan
