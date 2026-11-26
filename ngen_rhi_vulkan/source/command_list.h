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

    // What one frame records into: its slot's command buffer and the acquired swap chain image.
    struct FrameTarget
    {
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        SwapChainImage *image = nullptr;
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

        // Ends a frame's commands: closes a render pass left open (an Error message), and moves an image no
        // pass rendered to into the layout presentation needs.
        void finish();

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

    private:
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

        // Bound state lasts the whole frame (Vulkan keeps it across render passes in a command buffer).
        bool pipelineBound_ = false;
        std::vector<uint32_t> pipelineBindings_; // the vertex bindings the bound pipeline reads
        std::vector<uint32_t> boundBindings_;    // the bindings a vertex buffer is bound to
    };
} // namespace ngen::rhi::vulkan
