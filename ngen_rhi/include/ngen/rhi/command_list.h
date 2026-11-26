// Spec: rhi. Behaviour changes go through it.
#pragma once

#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/handle.h>

#include <cstdint>

namespace ngen::rhi
{
    // The recorder for one frame's commands. Misuse (a command outside its place, a null or stale handle)
    // skips the command and sends an Error message to the device's sink.
    //
    // The command list tracks the swap chain image's ResourceState: Undefined at beginFrame, and left in
    // `after` by each barrier on it. endFrame moves the image from its tracked state to Present, unless it is
    // already there.
    class CommandList
    {
    public:
        virtual ~CommandList() = default;

        // Makes every earlier use of the target in `before` finish before any later use in `after`, and
        // changes its state to `after`. Both ColorAttachment orders one pass's colour writes before the next's.
        // Skipped with an Error message inside beginRendering...endRendering, or when validate(barrier,
        // trackedState) fails.
        virtual void barrier(const Barrier &barrier) = 0;

        // Starts rendering to this frame's swap chain image, with the full image as render area; an image not
        // in ColorAttachment is first moved there from its tracked state, as a barrier would. In a GPU
        // backend each pass starts with the full image as viewport and scissor; a setViewport or setScissor
        // lasts until the end of its pass.
        virtual void beginRendering(const ColorTarget &target) = 0;
        virtual void endRendering() = 0;

        virtual void setViewport(float x, float y, float width, float height, float minDepth = 0,
                                 float maxDepth = 1) = 0;
        virtual void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        // bindPipeline, bindVertexBuffer and draw are recorded only between beginRendering and endRendering.
        virtual void bindPipeline(PipelineHandle pipeline) = 0;
        virtual void bindVertexBuffer(uint32_t binding, BufferHandle buffer, uint64_t offset = 0) = 0;
        virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0,
                          uint32_t firstInstance = 0) = 0;
    };
} // namespace ngen::rhi
