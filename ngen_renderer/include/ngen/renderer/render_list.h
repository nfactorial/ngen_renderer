// Spec: renderer. Behaviour changes go through it.
#pragma once

#include <ngen/rhi/handle.h>

#include <cstdint>
#include <span>
#include <vector>

namespace ngen::renderer
{
    struct StageId
    {
        uint32_t index;

        friend bool operator==(const StageId &, const StageId &) = default;
    };

    struct VertexBufferBinding
    {
        uint32_t binding;
        rhi::BufferHandle buffer;
        uint64_t offset = 0;
    };

    struct DrawItem
    {
        StageId stage;
        rhi::PipelineHandle pipeline;
        std::vector<VertexBufferBinding> vertexBuffers;
        uint32_t vertexCount;
        uint32_t instanceCount = 1;
        uint32_t firstVertex = 0;
        uint32_t firstInstance = 0;
    };

    // One frame's request: the draw items, in the order the game added them.
    class RenderList
    {
    public:
        void add(DrawItem item);
        void clear();
        [[nodiscard]] std::span<const DrawItem> items() const;

    private:
        std::vector<DrawItem> items_;
    };
} // namespace ngen::renderer
