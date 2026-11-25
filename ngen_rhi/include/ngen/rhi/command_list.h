// Spec: rhi. Behaviour changes go through it.
#pragma once

#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/handle.h>

#include <cstdint>

namespace ngen::rhi
{
    // The recorder for one frame's commands. Misuse (a command outside its place, a null or stale handle)
    // skips the command and sends an Error message to the device's sink.
    class CommandList
    {
    public:
        virtual ~CommandList() = default;

        // Starts rendering to this frame's swap chain image, with the full image as render area. The frame's
        // viewport and scissor default to the full image.
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
