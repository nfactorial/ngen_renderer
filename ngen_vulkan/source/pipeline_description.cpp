
#include <cstring>
#include "pipeline_description.h"
#include "render_pass.h"
#include "shader.h"

namespace {
    const char *kDefaultMainFunction = "main";

    template <typename TType> void ZeroItem(TType &item) {
        memset(&item, 0, sizeof(TType));
    }
}

namespace ngen::vulkan {
    PipelineDescription::PipelineDescription() {
        ZeroItem(m_colorBlendAttachment);
        ZeroItem(m_inputAssembly);
        ZeroItem(m_vertexInputInfo);
        ZeroItem(m_multisampling);
        ZeroItem(m_colorBlending);
        ZeroItem(m_rasterizer);
        ZeroItem(m_viewportState);
        ZeroItem(m_viewport);
        ZeroItem(m_scissor);

        m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

        m_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        m_rasterizer.lineWidth = 1.0f;
        m_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        m_rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        m_colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_colorBlending.logicOp = VK_LOGIC_OP_COPY;
        m_colorBlending.attachmentCount = 1;
        m_colorBlending.pAttachments = &m_colorBlendAttachment;
    }

    //! \brief Complete the configuration of pipeline information.
    //! \param renderPass [in] - The render pass the pipeline is associated with.
    //! \returns Description of the graphics pipeline described by the object.
    const VkGraphicsPipelineCreateInfo& PipelineDescription::finalize(const RenderPass &renderPass, VkPipelineLayout layout) {
        memset(&m_createInfo, 0, sizeof(m_createInfo));

        m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        m_viewportState.viewportCount = 1;          // TODO: Support multiple viewports
        m_viewportState.pViewports = &m_viewport;   //

        m_vertexInputInfo.pVertexAttributeDescriptions = m_attributes.data();
        m_vertexInputInfo.vertexAttributeDescriptionCount = m_attributes.size();

        m_vertexInputInfo.pVertexBindingDescriptions = m_bindings.data();
        m_vertexInputInfo.vertexBindingDescriptionCount = m_bindings.size();

        ZeroItem(m_createInfo);

        m_createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        m_createInfo.stageCount = m_shaderStages.size();
        m_createInfo.pStages = m_shaderStages.data();
        m_createInfo.pVertexInputState = &m_vertexInputInfo;
        m_createInfo.pInputAssemblyState = &m_inputAssembly;
        m_createInfo.pViewportState = &m_viewportState;
        m_createInfo.pRasterizationState = &m_rasterizer;
        m_createInfo.pMultisampleState = &m_multisampling;
        m_createInfo.pColorBlendState = &m_colorBlending;
        m_createInfo.layout = layout;
        m_createInfo.renderPass = renderPass;
        m_createInfo.subpass = 0;
        m_createInfo.basePipelineHandle = VK_NULL_HANDLE;

        return m_createInfo;
    }

    //! \brief Configures the viewport properties that are associated with the pipeline.
    //! \param x [in] - Offset within the display along the horizontal axis of the viewport.
    //! \param y [in] - Offset within the display along the vertical axis of the viewport.
    //! \param width [in] - Width (in pixels) of the viewport.
    //! \param height [in] - Height (in pixels) of the viewport.
    //! \param minDepth [in] - The minimum depth value of the viewport.
    //! \param maxDepth [in] - The maximum depth value of the viewport.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) {
        m_viewport.x = x;
        m_viewport.y = y;
        m_viewport.width = width;
        m_viewport.height = height;
        m_viewport.minDepth = minDepth;
        m_viewport.maxDepth = maxDepth;

        m_viewportState.viewportCount = 1;
        m_viewportState.pViewports = &m_viewport;

        return *this;
    }

    //! \brief Configures the scissor properties that are associated with the pipeline.
    //! \param x [in] - Offset within the viewport of the scissor.
    //! \param y [in] - Offset within the viewport of the scissor.
    //! \param extent [in] - Extents of the scissor rectangle.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::setScissor(int32_t x, int32_t y, const VkExtent2D &extent) {
        m_scissor.offset.x = x;
        m_scissor.offset.y = y;
        m_scissor.extent = extent;

        m_viewportState.scissorCount = 1;
        m_viewportState.pScissors = &m_scissor;

        return *this;
    }

    //! \brief Specifies the culling mode to be used by the pipeline.
    //! \param cullMode [in] - The cull mode to be used by the pipeline description.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::setCullMode(VkCullModeFlagBits cullMode) {
        m_rasterizer.cullMode = cullMode;
        return *this;
    }

    //! \brief Specifies the winding order considered to be front-facing by the pipeline.
    //! \param frontFace [in] - The winding order of the pipelines front-facing polygons.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::setFrontFace(VkFrontFace frontFace) {
        m_rasterizer.frontFace = frontFace;
        return *this;
    }

    //! \brief Specifies the rendering style applied to rasterized polygons within the pipeline.
    //! \param mode [in] - The rendering style to be used when drawing polygons.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::setPolygonMode(VkPolygonMode mode) {
        m_rasterizer.polygonMode = mode;
        return *this;
    }

    //! \brief Specifies the width of lines drawn by the pipeline.
    //! \param lineWidth [in] - The width (in pixels) of rendered lines.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::setLineWidth(float lineWidth) {
        m_rasterizer.lineWidth = lineWidth;
        return *this;
    }

    //! \brief Adds an input binding that spans each vertex.
    //! \param stride [in] - The number of bytes between successive vertex data.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addVertexBinding(uint32_t stride) {
        return addBinding(m_bindings.size(), stride, VK_VERTEX_INPUT_RATE_VERTEX);
    }

    //! \brief Adds an input binding that spans each instance.
    //! \param stride [in] - The number of bytes between successive instance data.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addInstanceBinding(uint32_t stride) {
        return addBinding(m_bindings.size(), stride, VK_VERTEX_INPUT_RATE_INSTANCE);
    }

    //! \brief Adds an input binding using the supplied rate.
    //! \param binding [in] - Binding index the input is associated with.
    //! \param stride [in] - The distance (in bytes) between each input.
    //! \param inputRate [in] - How frequent the data is bound.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate) {
        VkVertexInputBindingDescription description{};

        description.binding = binding;
        description.stride = stride;
        description.inputRate = inputRate;

        m_bindings.push_back(description);

        return *this;
    }

    //! \brief Adds a attribute to the vertex definition.
    //! \param binding [in] -
    //! \param location [in] - The location of the data within the shader program.
    //! \param format [in] - The format of the data.
    //! \param offset [in] - The offset of the data within the pipeline.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addAttribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
        VkVertexInputAttributeDescription description;

        description.binding = binding;
        description.location = location;
        description.format = format;
        description.offset = offset;

        m_attributes.push_back(description);

        return *this;
    }

    //! \brief Adds a vertex shader to the pipeline description, using the default 'main' function.
    //! \param shader [in] - The vertex shader to be added to the pipeline description.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addVertexShader(const Shader &shader) {
        return addVertexShader(shader, kDefaultMainFunction);
    }

    //! \brief Adds a vertex shader to the pipeline description, using a specified entry point.
    //! \param shader [in] - The vertex shader to be added to the pipeline description.
    //! \param mainFunc [in] - Name of the shaders entry point function to be executed.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addVertexShader(const Shader &shader, const char *mainFunc) {
        return addShader(shader, VK_SHADER_STAGE_VERTEX_BIT, mainFunc);
    }

    //! \brief Adds a fragment shader to the pipeline description, using the default 'main' function.
    //! \param shader [in] - The fragment shader to be added to the pipeline description.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addFragmentShader(const Shader &shader) {
        return addFragmentShader(shader, kDefaultMainFunction);
    }

    //! \brief Adds a fragment shader to the pipeline description, using a specified entry point.
    //! \param shader [in] - The fragment shader to be added to the pipeline description.
    //! \param mainFunc [in] - Name of the shaders entry point function to be executed.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addFragmentShader(const Shader &shader, const char *mainFunc) {
        return addShader(shader, VK_SHADER_STAGE_FRAGMENT_BIT, mainFunc);
    }

    //! \brief Adds a shader of a specified type to the pipeline description, using a specified entry point.
    //! \param shader [in] - The shader to be added to the pipeline description.
    //! \param flags [in] - Flags associated with the shader being added.
    //! \param mainFunc [in] - Name of the shaders entry point function to be executed.
    //! \returns Reference to self allowing for call chaining.
    PipelineDescription& PipelineDescription::addShader(const Shader &shader, VkShaderStageFlagBits flags, const char *mainFunc) {
        VkPipelineShaderStageCreateInfo stageInfo{};

        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = flags;
        stageInfo.module = shader;
        stageInfo.pName = mainFunc;

        m_shaderStages.push_back(stageInfo);

        return *this;
    }
}
