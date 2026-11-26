// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include "render_pass_cache.h"

#include <ngen/error.h>
#include <ngen/rhi/descriptions.h>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace ngen::rhi::vulkan
{
    // A shader is its SPIR-V words; modules are made from a pipeline's own copy when it needs them.
    struct VulkanShader
    {
        std::vector<uint32_t> code;
    };

    // One stage's copy of its shader's bytecode and entry point, so destroying the shader handle never
    // affects the pipeline.
    struct PipelineStage
    {
        std::vector<uint32_t> code;
        std::string entryPoint;
    };

    // A pipeline handle's resource: one VkPipeline per colour format it has been used with, each made against
    // that format's Clear/Store render pass, all sharing one layout. Move-only; destroying it destroys them.
    class Pipeline
    {
    public:
        // Makes the layout and the variant for initialFormat.
        [[nodiscard]] static std::expected<Pipeline, ngen::Error> create(VkDevice device, RenderPassCache &renderPasses,
                                                                         GraphicsPipelineDescription description,
                                                                         PipelineStage vertex, PipelineStage fragment,
                                                                         VkFormat initialFormat);

        Pipeline(Pipeline &&other) noexcept;
        Pipeline &operator=(Pipeline &&other) noexcept;
        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;
        ~Pipeline();

        // The variant for the colour format, created when the pipeline has none for it yet.
        [[nodiscard]] std::expected<VkPipeline, ngen::Error> variant(RenderPassCache &renderPasses, VkFormat format);

        [[nodiscard]] const GraphicsPipelineDescription &description() const
        {
            return description_;
        }

    private:
        struct Variant
        {
            VkFormat format;
            VkPipeline pipeline;
        };

        Pipeline(VkDevice device, GraphicsPipelineDescription description, PipelineStage vertex,
                 PipelineStage fragment);
        void release();

        VkDevice device_ = VK_NULL_HANDLE;
        GraphicsPipelineDescription description_;
        PipelineStage vertex_;
        PipelineStage fragment_;
        VkPipelineLayout layout_ = VK_NULL_HANDLE;
        std::vector<Variant> variants_;
    };
} // namespace ngen::rhi::vulkan
