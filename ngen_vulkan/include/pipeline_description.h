#ifndef NGEN_VULKAN_PIPELINE_DESCRIPTION
#define NGEN_VULKAN_PIPELINE_DESCRIPTION

#include <vector>
#include <vulkan/vulkan.h>

namespace ngen::vulkan {
    class Shader;
    class RenderPass;

    //! \brief Provides support methods for declaring the structure of a pipeline object.
    //! \remarks PipelineDescription is intended to be short lived, used when constructing a Pipeline object and then
    //!   discarded. Once you have created the Pipeline object, you should discard the PipelineDescription object as
    //!   it will no longer be required.
    class PipelineDescription {
    public:
        PipelineDescription();
        ~PipelineDescription() = default;

        PipelineDescription(const PipelineDescription &other) = default;
        PipelineDescription& operator=(const PipelineDescription &other) = default;

        [[nodiscard]] const VkGraphicsPipelineCreateInfo& finalize(const RenderPass &renderPass, VkPipelineLayout layout);

        PipelineDescription& setViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
        PipelineDescription& setScissor(int32_t x, int32_t y, const VkExtent2D &extent);

        PipelineDescription& addVertexBinding(uint32_t stride);
        PipelineDescription& addInstanceBinding(uint32_t stride);

        PipelineDescription& setCullMode(VkCullModeFlagBits cullMode);
        PipelineDescription& setFrontFace(VkFrontFace frontFace);
        PipelineDescription& setPolygonMode(VkPolygonMode mode);
        PipelineDescription& setLineWidth(float lineWidth);

        // Vertex declaration
        PipelineDescription& addAttribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);

        PipelineDescription& addVertexShader(const Shader &shader);
        PipelineDescription& addVertexShader(const Shader &shader, const char *mainFunc);

        PipelineDescription& addFragmentShader(const Shader &shader);
        PipelineDescription& addFragmentShader(const Shader &shader, const char *mainFunc);

    private:
        PipelineDescription& addShader(const Shader &shader, VkShaderStageFlagBits flags, const char *mainFunc);
        PipelineDescription& addBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);

    private:
        std::vector<VkVertexInputAttributeDescription> m_attributes;
        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
        std::vector<VkVertexInputBindingDescription> m_bindings;

        VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
        VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
        VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
        VkPipelineMultisampleStateCreateInfo m_multisampling;
        VkPipelineColorBlendStateCreateInfo m_colorBlending;
        VkPipelineRasterizationStateCreateInfo m_rasterizer;
        VkPipelineViewportStateCreateInfo m_viewportState;
        VkViewport m_viewport;
        VkRect2D m_scissor;

        VkGraphicsPipelineCreateInfo m_createInfo;
    };
}

#endif //NGEN_VULKAN_PIPELINE_DESCRIPTION
