// Spec: rhi-vulkan. Behaviour changes go through it.
#include "pipeline.h"

#include "vulkan_result.h"

#include <array>
#include <utility>

namespace ngen::rhi::vulkan
{
    namespace
    {
        VkFormat toVulkan(const Format format)
        {
            switch (format)
            {
            case Format::R32G32Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::R32G32B32Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::R32G32B32A32Float:
                break;
            }
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        }

        VkPrimitiveTopology toVulkan(const Topology topology)
        {
            switch (topology)
            {
            case Topology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case Topology::TriangleStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case Topology::LineList:
                break;
            }
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }

        VkCullModeFlags toVulkan(const CullMode cull)
        {
            switch (cull)
            {
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:
                break;
            }
            return VK_CULL_MODE_BACK_BIT;
        }

        VkFrontFace toVulkan(const FrontFace frontFace)
        {
            return frontFace == FrontFace::Clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }

        VkPolygonMode toVulkan(const PolygonMode mode)
        {
            return mode == PolygonMode::Fill ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
        }

        // A shader module that lives only while a variant is being created.
        class ShaderModule
        {
        public:
            ShaderModule(const VkDevice device, const std::vector<uint32_t> &code) : device_(device)
            {
                VkShaderModuleCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                info.codeSize = code.size() * sizeof(uint32_t);
                info.pCode = code.data();
                result_ = vkCreateShaderModule(device_, &info, nullptr, &module_);
                if (result_ != VK_SUCCESS)
                {
                    module_ = VK_NULL_HANDLE;
                }
            }

            ~ShaderModule()
            {
                if (module_ != VK_NULL_HANDLE)
                {
                    vkDestroyShaderModule(device_, module_, nullptr);
                }
            }

            ShaderModule(const ShaderModule &) = delete;
            ShaderModule &operator=(const ShaderModule &) = delete;

            [[nodiscard]] VkResult result() const
            {
                return result_;
            }

            [[nodiscard]] VkShaderModule get() const
            {
                return module_;
            }

        private:
            VkDevice device_;
            VkShaderModule module_ = VK_NULL_HANDLE;
            VkResult result_;
        };
    } // namespace

    std::expected<Pipeline, ngen::Error> Pipeline::create(const VkDevice device, RenderPassCache &renderPasses,
                                                          GraphicsPipelineDescription description,
                                                          PipelineStage vertex, PipelineStage fragment,
                                                          const VkFormat initialFormat)
    {
        Pipeline pipeline(device, std::move(description), std::move(vertex), std::move(fragment));

        // No descriptor sets or push constants in this milestone.
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (const VkResult result = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipeline.layout_);
            result != VK_SUCCESS)
        {
            pipeline.layout_ = VK_NULL_HANDLE;
            return std::unexpected(vulkanError("vkCreatePipelineLayout", result));
        }

        if (auto created = pipeline.variant(renderPasses, initialFormat); !created)
        {
            return std::unexpected(std::move(created.error()));
        }
        return pipeline;
    }

    Pipeline::Pipeline(const VkDevice device, GraphicsPipelineDescription description, PipelineStage vertex,
                       PipelineStage fragment)
        : device_(device), description_(std::move(description)), vertex_(std::move(vertex)),
          fragment_(std::move(fragment))
    {
    }

    Pipeline::Pipeline(Pipeline &&other) noexcept
        : device_(std::exchange(other.device_, VK_NULL_HANDLE)), description_(std::move(other.description_)),
          vertex_(std::move(other.vertex_)), fragment_(std::move(other.fragment_)),
          layout_(std::exchange(other.layout_, VK_NULL_HANDLE)), variants_(std::exchange(other.variants_, {}))
    {
    }

    Pipeline &Pipeline::operator=(Pipeline &&other) noexcept
    {
        if (this != &other)
        {
            release();
            device_ = std::exchange(other.device_, VK_NULL_HANDLE);
            description_ = std::move(other.description_);
            vertex_ = std::move(other.vertex_);
            fragment_ = std::move(other.fragment_);
            layout_ = std::exchange(other.layout_, VK_NULL_HANDLE);
            variants_ = std::exchange(other.variants_, {});
        }
        return *this;
    }

    Pipeline::~Pipeline()
    {
        release();
    }

    void Pipeline::release()
    {
        for (auto variant = variants_.rbegin(); variant != variants_.rend(); ++variant)
        {
            vkDestroyPipeline(device_, variant->pipeline, nullptr);
        }
        variants_.clear();
        if (layout_ != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device_, layout_, nullptr);
            layout_ = VK_NULL_HANDLE;
        }
    }

    std::expected<VkPipeline, ngen::Error> Pipeline::variant(RenderPassCache &renderPasses, const VkFormat format)
    {
        for (const Variant &variant : variants_)
        {
            if (variant.format == format)
            {
                return variant.pipeline;
            }
        }

        const auto renderPass = renderPasses.compatible(format);
        if (!renderPass)
        {
            return std::unexpected(renderPass.error());
        }

        const ShaderModule vertexModule(device_, vertex_.code);
        if (vertexModule.result() != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkCreateShaderModule (vertex)", vertexModule.result()));
        }
        const ShaderModule fragmentModule(device_, fragment_.code);
        if (fragmentModule.result() != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkCreateShaderModule (fragment)", fragmentModule.result()));
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> stages{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vertexModule.get();
        stages[0].pName = vertex_.entryPoint.c_str();
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fragmentModule.get();
        stages[1].pName = fragment_.entryPoint.c_str();

        std::vector<VkVertexInputBindingDescription> bindings;
        for (const VertexBinding &binding : description_.bindings)
        {
            bindings.push_back(VkVertexInputBindingDescription{
                binding.binding, binding.stride,
                binding.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX});
        }
        std::vector<VkVertexInputAttributeDescription> attributes;
        for (const VertexAttribute &attribute : description_.attributes)
        {
            attributes.push_back(VkVertexInputAttributeDescription{attribute.location, attribute.binding,
                                                                   toVulkan(attribute.format), attribute.offset});
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
        vertexInput.pVertexBindingDescriptions = bindings.data();
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        vertexInput.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = toVulkan(description_.topology);
        // Strips restart on the all-ones index, matching Metal, which cannot turn restart off. Core
        // Vulkan forbids restart on list topologies, so it stays off for them.
        inputAssembly.primitiveRestartEnable = description_.topology == Topology::TriangleStrip ? VK_TRUE : VK_FALSE;

        // Viewport and scissor are dynamic; only their counts are fixed.
        VkPipelineViewportStateCreateInfo viewport{};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterization{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.polygonMode = toVulkan(description_.polygonMode);
        rasterization.cullMode = toVulkan(description_.cull);
        rasterization.frontFace = toVulkan(description_.frontFace);
        rasterization.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo blend{};
        blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend.attachmentCount = 1;
        blend.pAttachments = &blendAttachment;

        const std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamic.pDynamicStates = dynamicStates.data();

        VkGraphicsPipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.stageCount = static_cast<uint32_t>(stages.size());
        createInfo.pStages = stages.data();
        createInfo.pVertexInputState = &vertexInput;
        createInfo.pInputAssemblyState = &inputAssembly;
        createInfo.pViewportState = &viewport;
        createInfo.pRasterizationState = &rasterization;
        createInfo.pMultisampleState = &multisample;
        createInfo.pColorBlendState = &blend;
        createInfo.pDynamicState = &dynamic;
        createInfo.layout = layout_;
        createInfo.renderPass = *renderPass;
        createInfo.subpass = 0;

        VkPipeline created = VK_NULL_HANDLE;
        if (const VkResult result =
                vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &createInfo, nullptr, &created);
            result != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkCreateGraphicsPipelines", result));
        }
        variants_.push_back(Variant{format, created});
        return created;
    }
} // namespace ngen::rhi::vulkan
