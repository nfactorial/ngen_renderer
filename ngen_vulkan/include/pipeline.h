
#ifndef NGEN_VULKAN_PIPELINE
#define NGEN_VULKAN_PIPELINE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class PipelineDescription;
    class VulkanContext;
    class RenderPass;
    class Shader;

    class Pipeline {
    public:
        Pipeline();
        ~Pipeline();

        void dispose();
        [[nodiscard]] bool create(const PipelineDescription &description);

        [[nodiscard]] bool initialize(const VulkanContext &context, const RenderPass &renderPass, const VkExtent2D &extent, const Shader &vertexShader, const Shader &fragmentShader);

        [[nodiscard]] VkPipelineLayout getLayout() const;
        [[nodiscard]] VkPipeline getPipeline() const;

        void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint) const;

    private:
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
