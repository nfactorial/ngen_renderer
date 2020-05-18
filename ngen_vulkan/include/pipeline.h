
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
        [[nodiscard]] bool create(const VulkanContext &context, const RenderPass &renderPass, PipelineDescription &description);

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
