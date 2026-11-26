// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include "swap_chain.h"

#include <ngen/rhi/command_list.h>
#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/handle.h>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace ngen::rhi::vulkan
{
    class DeviceImpl;

    // How an image is used in a ResourceState: the layout, pipeline stage and access mask a barrier maps the state
    // to.
    struct ImageStateUse
    {
        VkImageLayout layout;
        VkPipelineStageFlags stage;
        VkAccessFlags access;
    };

    // The use a barrier maps a state to. Present is PRESENT_SRC_KHR on a device that presents, and
    // COLOR_ATTACHMENT_OPTIMAL on one that never does (PRESENT_SRC_KHR needs VK_KHR_swapchain).
    [[nodiscard]] ImageStateUse imageStateUse(ResourceState state, bool presents);

    // The load op of the render pass beginRendering uses: a Load while the image is Undefined is DontCare,
    // because its contents are undefined; every other load op is used as requested.
    [[nodiscard]] LoadOp passLoad(LoadOp requested, ResourceState imageState);

    // What one frame records into: its slot's command buffer and the acquired swap chain image.
    struct FrameTarget
    {
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        SwapChainImageResources *image = nullptr;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkExtent2D extent{0, 0};
    };

    // Records one frame's commands into its command buffer, after checking call order and handles. Misuse
    // skips the command and sends an Error message to the device's sink.
    class VulkanCommandList final : public CommandList
    {
    public:
        explicit VulkanCommandList(DeviceImpl &device);

        // Starts a frame; the command buffer is already recording.
        void begin(const FrameTarget &target);

        // Ends a frame's commands: closes a render pass left open (an Error message), and moves the image from
        // its tracked state to Present unless it is already there.
        void finish();

        void barrier(const Barrier &barrier) override;
        void beginRendering(const ColorTarget &target) override;
        void endRendering() override;
        void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) override;
        void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;
        void bindPipeline(PipelineHandle pipeline) override;
        void bindVertexBuffer(uint32_t binding, BufferHandle buffer, uint64_t offset) override;
        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

        // The viewport and scissor most recently recorded into the command buffer (for tests).
        [[nodiscard]] const VkViewport &viewport() const
        {
            return viewport_;
        }

        [[nodiscard]] const VkRect2D &scissor() const
        {
            return scissor_;
        }

        // The vkCmdPipelineBarrier calls recorded in the frame being recorded, or in the last one after its
        // endFrame (for tests).
        [[nodiscard]] uint32_t pipelineBarrierCount() const
        {
            return pipelineBarrierCount_;
        }

    private:
        // A barrier target's image, and the state the command list tracks for it.
        struct TrackedImage
        {
            VkImage image;
            ResourceState *state;
        };

        [[nodiscard]] TrackedImage tracked(const BarrierTarget &target);

        // Records one vkCmdPipelineBarrier with one image barrier moving the image from before to after, and
        // leaves the tracked state in after.
        void transition(const TrackedImage &image, ResourceState after);

        // Record the state and remember it.
        void applyViewport(const VkViewport &viewport);
        void applyScissor(const VkRect2D &scissor);

        bool requireRecording(const char *command);
        bool requireRendering(const char *command);

        DeviceImpl &device_;
        FrameTarget target_;
        bool recording_ = false;
        bool rendering_ = false;
        VkViewport viewport_{};
        VkRect2D scissor_{};
        ResourceState imageState_ = ResourceState::Undefined; // the swap chain image's tracked state
        uint32_t pipelineBarrierCount_ = 0;

        // Bound state lasts the whole frame (Vulkan keeps it across render passes in a command buffer).
        bool pipelineBound_ = false;
        std::vector<uint32_t> pipelineBindings_; // the vertex bindings the bound pipeline reads
        std::vector<uint32_t> boundBindings_;    // the bindings a vertex buffer is bound to
    };
} // namespace ngen::rhi::vulkan
