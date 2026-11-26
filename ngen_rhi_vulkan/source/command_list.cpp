// Spec: rhi-vulkan. Behaviour changes go through it.
#include "command_list.h"

#include "device_impl.h"

#include <algorithm>
#include <string>
#include <variant>

namespace ngen::rhi::vulkan
{
    ImageStateUse imageStateUse(const ResourceState state, const bool presents)
    {
        switch (state)
        {
        case ResourceState::Undefined:
            // A barrier from Undefined waits at the colour output stage, where the acquire semaphore's wait is.
            return ImageStateUse{VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0};
        case ResourceState::ColorAttachment:
            return ImageStateUse{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
        case ResourceState::ShaderRead:
            return ImageStateUse{VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 VK_ACCESS_SHADER_READ_BIT};
        case ResourceState::Present:
            break;
        }
        return ImageStateUse{presents ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0};
    }

    LoadOp passLoad(const LoadOp requested, const ResourceState imageState)
    {
        // While the image is Undefined its contents are undefined, so loading them is the same as not caring
        // about them.
        return requested == LoadOp::Load && imageState == ResourceState::Undefined ? LoadOp::DontCare : requested;
    }

    VulkanCommandList::VulkanCommandList(DeviceImpl &device) : device_(device)
    {
    }

    void VulkanCommandList::begin(const FrameTarget &target)
    {
        target_ = target;
        recording_ = true;
        rendering_ = false;
        pipelineBound_ = false;
        pipelineBindings_.clear();
        boundBindings_.clear();
        // The image keeps no record of being rendered across frames: whatever it holds may be discarded.
        imageState_ = ResourceState::Undefined;
        pipelineBarrierCount_ = 0;
    }

    VulkanCommandList::TrackedImage VulkanCommandList::tracked(const BarrierTarget &target)
    {
        return std::visit(
            [this](const SwapChainImage &) { return TrackedImage{target_.image->image, &imageState_}; }, target);
    }

    void VulkanCommandList::transition(const TrackedImage &image, const ResourceState after)
    {
        const bool presents = device_.context().windowExpected();
        const ImageStateUse from = imageStateUse(*image.state, presents);
        const ImageStateUse to = imageStateUse(after, presents);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = from.access;
        barrier.dstAccessMask = to.access;
        barrier.oldLayout = from.layout;
        barrier.newLayout = to.layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(target_.commandBuffer, from.stage, to.stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        ++pipelineBarrierCount_;
        *image.state = after;
    }

    void VulkanCommandList::applyViewport(const VkViewport &viewport)
    {
        vkCmdSetViewport(target_.commandBuffer, 0, 1, &viewport);
        viewport_ = viewport;
    }

    void VulkanCommandList::applyScissor(const VkRect2D &scissor)
    {
        vkCmdSetScissor(target_.commandBuffer, 0, 1, &scissor);
        scissor_ = scissor;
    }

    void VulkanCommandList::finish()
    {
        if (rendering_)
        {
            device_.report(Severity::Error, "endFrame: beginRendering has no matching endRendering");
            vkCmdEndRenderPass(target_.commandBuffer);
            rendering_ = false;
        }

        // Presenting needs the image in Present; in a frame that drew nothing it moves there from Undefined.
        if (imageState_ != ResourceState::Present)
        {
            transition(tracked(SwapChainImage{}), ResourceState::Present);
        }
        recording_ = false;
    }

    bool VulkanCommandList::requireRecording(const char *const command)
    {
        if (recording_)
        {
            return true;
        }
        device_.report(Severity::Error, std::string(command) + " called after endFrame");
        return false;
    }

    bool VulkanCommandList::requireRendering(const char *const command)
    {
        if (rendering_)
        {
            return true;
        }
        device_.report(Severity::Error, std::string(command) + ": called outside beginRendering and endRendering");
        return false;
    }

    void VulkanCommandList::barrier(const Barrier &barrier)
    {
        if (!requireRecording("barrier"))
        {
            return;
        }
        // A pipeline barrier inside a render pass would need the pass to declare a self-dependency.
        if (rendering_)
        {
            device_.report(Severity::Error, "barrier: called inside beginRendering and endRendering");
            return;
        }
        const TrackedImage image = tracked(barrier.target);
        if (auto valid = validate(barrier, *image.state); !valid)
        {
            device_.report(Severity::Error, valid.error().message);
            return;
        }
        transition(image, barrier.after);
    }

    void VulkanCommandList::beginRendering(const ColorTarget &target)
    {
        if (!requireRecording("beginRendering"))
        {
            return;
        }
        if (rendering_)
        {
            device_.report(Severity::Error, "beginRendering: already rendering");
            return;
        }

        const auto renderPass = device_.renderPasses().get(target_.format, passLoad(target.load, imageState_),
                                                           target.store);
        if (!renderPass)
        {
            device_.report(Severity::Error, "beginRendering: " + renderPass.error().message);
            return;
        }

        // Render passes start with their image a colour attachment and never change its layout.
        if (imageState_ != ResourceState::ColorAttachment)
        {
            transition(tracked(SwapChainImage{}), ResourceState::ColorAttachment);
        }

        VkClearValue clear{};
        std::copy(target.clearColor.begin(), target.clearColor.end(), clear.color.float32);

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = *renderPass;
        beginInfo.framebuffer = target_.image->framebuffer;
        beginInfo.renderArea = VkRect2D{{0, 0}, target_.extent};
        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = &clear;
        vkCmdBeginRenderPass(target_.commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Every pass starts with the full image as viewport and scissor, whatever an earlier pass (or a call
        // before this one) set: a setViewport or setScissor lasts only until the end of its pass.
        applyViewport(VkViewport{0.0f, 0.0f, static_cast<float>(target_.extent.width),
                                 static_cast<float>(target_.extent.height), 0.0f, 1.0f});
        applyScissor(VkRect2D{{0, 0}, target_.extent});

        rendering_ = true;
    }

    void VulkanCommandList::endRendering()
    {
        if (!requireRecording("endRendering"))
        {
            return;
        }
        if (!rendering_)
        {
            device_.report(Severity::Error, "endRendering: not rendering");
            return;
        }
        vkCmdEndRenderPass(target_.commandBuffer);
        rendering_ = false;
    }

    void VulkanCommandList::setViewport(const float x, const float y, const float width, const float height,
                                        const float minDepth, const float maxDepth)
    {
        if (!requireRecording("setViewport"))
        {
            return;
        }
        applyViewport(VkViewport{x, y, width, height, minDepth, maxDepth});
    }

    void VulkanCommandList::setScissor(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
    {
        if (!requireRecording("setScissor"))
        {
            return;
        }
        // Vulkan requires a scissor offset of 0 or more.
        if (x < 0 || y < 0)
        {
            device_.report(Severity::Error, "setScissor: x and y must not be negative");
            return;
        }
        applyScissor(VkRect2D{{x, y}, {width, height}});
    }

    void VulkanCommandList::bindPipeline(const PipelineHandle pipeline)
    {
        if (!requireRecording("bindPipeline") || !requireRendering("bindPipeline"))
        {
            return;
        }
        Pipeline *target = device_.findPipeline(pipeline);
        if (target == nullptr)
        {
            device_.report(Severity::Error, "bindPipeline: the pipeline handle is null or stale");
            return;
        }
        // A swap chain recreation may have changed the format; the variant for it is made on first use.
        const auto variant = target->variant(device_.renderPasses(), target_.format);
        if (!variant)
        {
            device_.report(Severity::Error, "bindPipeline: " + variant.error().message);
            return;
        }
        vkCmdBindPipeline(target_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *variant);

        pipelineBound_ = true;
        pipelineBindings_.clear();
        for (const VertexBinding &binding : target->description().bindings)
        {
            pipelineBindings_.push_back(binding.binding);
        }
    }

    void VulkanCommandList::bindVertexBuffer(const uint32_t binding, const BufferHandle buffer, const uint64_t offset)
    {
        if (!requireRecording("bindVertexBuffer") || !requireRendering("bindVertexBuffer"))
        {
            return;
        }
        const Buffer *target = device_.findBuffer(buffer);
        if (target == nullptr)
        {
            device_.report(Severity::Error, "bindVertexBuffer: the buffer handle is null or stale");
            return;
        }
        if ((target->usage() & BufferUsage::Vertex) == static_cast<BufferUsage>(0))
        {
            device_.report(Severity::Error, "bindVertexBuffer: the buffer was not created with BufferUsage::Vertex");
            return;
        }
        if (offset >= target->size())
        {
            device_.report(Severity::Error, "bindVertexBuffer: the offset is not inside the buffer");
            return;
        }
        if (binding >= device_.context().limits().maxVertexInputBindings)
        {
            device_.report(Severity::Error, "bindVertexBuffer: the binding is past the device's maxVertexInputBindings");
            return;
        }

        const VkBuffer handle = target->handle();
        const VkDeviceSize vulkanOffset = offset;
        vkCmdBindVertexBuffers(target_.commandBuffer, binding, 1, &handle, &vulkanOffset);
        if (std::ranges::find(boundBindings_, binding) == boundBindings_.end())
        {
            boundBindings_.push_back(binding);
        }
    }

    void VulkanCommandList::draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
                                 const uint32_t firstInstance)
    {
        if (!requireRecording("draw") || !requireRendering("draw"))
        {
            return;
        }
        // Drawing with no pipeline, or without a buffer on a binding the pipeline reads, is invalid in Vulkan.
        if (!pipelineBound_)
        {
            device_.report(Severity::Error, "draw: no pipeline is bound");
            return;
        }
        for (const uint32_t binding : pipelineBindings_)
        {
            if (std::ranges::find(boundBindings_, binding) == boundBindings_.end())
            {
                device_.report(Severity::Error, "draw: the bound pipeline reads vertex binding " +
                                                    std::to_string(binding) + ", which has no buffer bound");
                return;
            }
        }
        vkCmdDraw(target_.commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
} // namespace ngen::rhi::vulkan
