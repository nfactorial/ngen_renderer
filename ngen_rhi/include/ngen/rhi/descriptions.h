// Spec: rhi. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/rhi/handle.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace ngen::rhi
{
    // A flag set: Vertex | Uniform is allowed.
    enum class BufferUsage : uint32_t
    {
        Vertex = 1,
        Uniform = 2,
    };

    constexpr BufferUsage operator|(const BufferUsage a, const BufferUsage b)
    {
        return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr BufferUsage operator&(const BufferUsage a, const BufferUsage b)
    {
        return static_cast<BufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    // Buffers are host-visible in this milestone.
    struct BufferDescription
    {
        uint64_t size;
        BufferUsage usage;
    };

    enum class ShaderStage
    {
        Vertex,
        Fragment,
    };

    // Bytecode in the format the backend consumes (SPIR-V for Vulkan); createShader copies the span.
    struct ShaderDescription
    {
        std::span<const std::byte> bytecode;
    };

    // Vertex attribute formats.
    enum class Format
    {
        R32G32Float,
        R32G32B32Float,
        R32G32B32A32Float,
    };

    struct VertexAttribute
    {
        uint32_t location;
        uint32_t binding;
        Format format;
        uint32_t offset;
    };

    struct VertexBinding
    {
        uint32_t binding;
        uint32_t stride;
        bool perInstance = false;
    };

    struct ShaderEntry
    {
        ShaderHandle shader;
        std::string entryPoint;
    };

    enum class Topology
    {
        TriangleList,
        TriangleStrip,
        LineList,
    };

    enum class CullMode
    {
        None,
        Front,
        Back,
    };

    enum class FrontFace
    {
        Clockwise,
        CounterClockwise,
    };

    enum class PolygonMode
    {
        Fill,
        Line,
    };

    // The pipeline renders to the swap chain's colour format; viewport and scissor are set while recording.
    struct GraphicsPipelineDescription
    {
        ShaderEntry vertex;
        ShaderEntry fragment;
        std::vector<VertexBinding> bindings;
        std::vector<VertexAttribute> attributes;
        Topology topology = Topology::TriangleList;
        CullMode cull = CullMode::Back;
        FrontFace frontFace = FrontFace::Clockwise;
        PolygonMode polygonMode = PolygonMode::Fill;
    };

    enum class LoadOp
    {
        Load,
        Clear,
        DontCare,
    };

    enum class StoreOp
    {
        Store,
        DontCare,
    };

    struct ColorTarget
    {
        LoadOp load = LoadOp::Clear;
        StoreOp store = StoreOp::Store;
        std::array<float, 4> clearColor = {0, 0, 0, 1};

        friend bool operator==(const ColorTarget &, const ColorTarget &) = default;
    };

    // Check the *Descriptions* requirements; each failure is InvalidArgument naming the field. Shared by
    // every Device implementation. Whether a pipeline's shader handles are live is the device's to check.
    [[nodiscard]] std::expected<void, ngen::Error> validate(const BufferDescription &description);
    [[nodiscard]] std::expected<void, ngen::Error> validate(const ShaderDescription &description);
    [[nodiscard]] std::expected<void, ngen::Error> validate(const GraphicsPipelineDescription &description);
} // namespace ngen::rhi
