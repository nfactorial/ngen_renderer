#include "pipeline.h"
#include "pipeline_description.h"
#include "vulkan_context.h"
#include "vulkan_error.h"
#include "render_pass.h"
#include "shader.h"

namespace ngen::vulkan {
    Pipeline::Pipeline()
    : m_layout(VK_NULL_HANDLE)
    , m_pipeline(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {
    }

    Pipeline::~Pipeline() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void Pipeline::dispose() {
        if (VK_NULL_HANDLE != m_pipeline) {
            vkDestroyPipeline(m_device, m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != m_layout) {
            vkDestroyPipelineLayout(m_device, m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }

        m_device = VK_NULL_HANDLE;
    }

    //! \brief Adds a descriptor set to the pipeline information, must be called before the pipeline is created.
    //! \param descriptorSet [in] - The descriptor set to be added to the pipeline definition.
    //! \returns <em>True</em> if the descriptor set was added successfully otherwise <em>false</em>.
    bool Pipeline::addDescriptorSet(VkDescriptorSetLayout descriptorSet) {
        if (VK_NULL_HANDLE != m_layout) {
            printf("Cannot add descriptor set after pipeline has been created.\n");
            return false;
        }

        if (VK_NULL_HANDLE == descriptorSet) {
            printf("A valid descriptor set must be provided.");
            return false;
        }

        m_descriptorSets.push_back(descriptorSet);
        return true;
    }

    //! \brief Creates the pipeline using the supplied description.
    //! \param context [in] - The vulkan context the pipeline will belong to.
    //! \param renderPass [in] - The render pass to be used with the pipeline.
    //! \param createInfo [in] - Description of the pipeline to be created.
    //! \returns True if the pipeline initialized successfully otherwise false.
    bool Pipeline::create(const VulkanContext &context, const RenderPass &renderPass, PipelineDescription &description) {
        if (m_device != VK_NULL_HANDLE) {
            printf("Pipeline already initialized\n");
            return false;
        }

        m_device = context.getDevice();

        // TODO: See createGraphicsPipeline in: https://vulkan-tutorial.com/code/14_command_buffers.cpp
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = m_descriptorSets.size();
        pipelineLayoutInfo.pSetLayouts = m_descriptorSets.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
            printf("Failed to create pipeline layout");

            dispose();
            return false;
        }

        const auto result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &description.finalize(renderPass, m_layout), nullptr, &m_pipeline);
        if (result != VK_SUCCESS) {
            printf("Failed to create pipeline with supplied description: %s\n", getResultString(result));

            dispose();
            return false;
        }

        return true;
    }

    //! \brief Binds the pipeline to the specified command buffer.
    //! \param commandBuffer [in] - The command buffer the pipeline should be bound to.
    //! \param pipelineBindPoint [in] - The bind point for the pipeline
    void Pipeline::bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint) const {
        vkCmdBindPipeline(commandBuffer, pipelineBindPoint, m_pipeline);
    }
}
