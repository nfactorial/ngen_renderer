
#ifndef NGEN_VULKAN_PIPELINE
#define NGEN_VULKAN_PIPELINE

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class PipelineDescription;
    class VulkanContext;
    class RenderPass;
    class Shader;

    //! \brief Provides access to the creation and management of a vulkan pipeline object.
    //! \remarks To define a pipeline, please make use of the helper class PipelineDescription. Which will allow you
    //!   to define a pipeline.
    class Pipeline {
    public:
        Pipeline();
        ~Pipeline();

        void dispose();
        [[nodiscard]] bool create(const VulkanContext &context, const RenderPass &renderPass, PipelineDescription &description);

        [[nodiscard]] VkPipelineLayout getLayout() const;
        [[nodiscard]] VkPipeline getPipeline() const;

        void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint) const;

        [[nodiscard]] bool addDescriptorSet(VkDescriptorSetLayout descriptorSet);

        operator VkPipeline() const;    // NOLINT

    private:
        std::vector<VkDescriptorSetLayout> m_descriptorSets;
        VkPipelineLayout m_layout;
        VkPipeline m_pipeline;

        VkDevice m_device;
    };

    inline Pipeline::operator VkPipeline() const {
        return m_pipeline;
    }

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
