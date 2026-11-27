// Spec: render-graph. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/rhi/command_list.h>
#include <ngen/rhi/descriptions.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ngen::renderer
{
    struct ResourceId
    {
        uint32_t index;

        friend bool operator==(const ResourceId &, const ResourceId &) = default;
    };

    // The pass's colour target, cleared to `clear` when it is given. `target` starts as an id no graph issues,
    // so a pass that never set it fails to compile.
    struct ColorWrite
    {
        ResourceId target{UINT32_MAX};
        std::optional<std::array<float, 4>> clear;
    };

    struct PassDescription
    {
        std::string name;
        int32_t order = 0;
        ColorWrite color;
        std::vector<ResourceId> reads;
        std::function<void(rhi::CommandList &)> record;
    };

    // The barriers recorded before the pass, and its colour target.
    struct CompiledPass
    {
        std::string name;
        std::vector<rhi::Barrier> barriers;
        rhi::ColorTarget colorTarget;
    };

    struct CompiledGraph
    {
        std::vector<CompiledPass> passes;
    };

    // Owns one frame's passes and resources, and turns their declarations into an ordered frame with every
    // barrier and load op decided. Filled, compiled or executed, then cleared for the next frame.
    class RenderGraph
    {
    public:
        ResourceId importTarget(rhi::BarrierTarget target, rhi::ResourceState current);
        void addPass(PassDescription pass);

        // Pure: compiling twice gives equal results.
        [[nodiscard]] std::expected<CompiledGraph, ngen::Error> compile() const;

        // Compiles, then records each pass's barriers, beginRendering, its callback and endRendering. When
        // compiling fails nothing is recorded.
        [[nodiscard]] std::expected<void, ngen::Error> execute(rhi::CommandList &commands) const;

        // Removes every pass and resource.
        void clear();

    private:
        struct Resource
        {
            rhi::BarrierTarget target;
            rhi::ResourceState state;
        };

        // Indices into passes_ in run order: ascending `order`, then the order added.
        [[nodiscard]] std::vector<size_t> runOrder() const;

        std::vector<Resource> resources_;
        std::vector<PassDescription> passes_;
    };
} // namespace ngen::renderer
