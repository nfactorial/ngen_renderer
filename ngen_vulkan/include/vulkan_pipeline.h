
#ifndef NGEN_VULKAN_PIPELINE
#define NGEN_VULKAN_PIPELINE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class VulkanContext;
    class RenderPass;
    class Shader;

    class Pipeline {
    public:
        Pipeline();
        ~Pipeline();

        void dispose();
        [[nodiscard]] bool initialize(const VulkanContext &context, const RenderPass &renderPass, const VkExtent2D &extent, const Shader &vertexShader, const Shader &fragmentShader);

        [[nodiscard]] VkPipelineLayout getLayout() const;
        [[nodiscard]] VkPipeline getPipeline() const;

        void bind(VkCommandBuffer commandBuffer) const;

    private:
        VkPipelineViewportStateCreateInfo m_viewportState;
        VkPipelineLayoutCreateInfo m_createInfo;
        VkViewport m_viewport;
        VkRect2D m_scissor;

        VkPipelineLayout m_layout;
        VkPipeline m_pipeline;

        VkDevice m_device;
    };

    inline VkPipeline Pipeline::getPipeline() const {
        return m_pipeline;
    }

    //! \brief Conversion operator for VKPipelineLayout
    inline VkPipelineLayout Pipeline::getLayout() const {
        return m_layout;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_PIPELINE
