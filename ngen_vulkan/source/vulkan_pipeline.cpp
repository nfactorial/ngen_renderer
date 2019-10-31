#include "vulkan_pipeline.h"
#include "device.h"
#include "vulkan_error.h"

namespace ngen::vulkan {
    Pipeline::Pipeline()
    : m_instance(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {
    }

    Pipeline::~Pipeline() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void Pipeline::dispose() {
        if (VK_NULL_HANDLE != m_instance) {
            vkDestroyPipelineLayout(m_device, m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    //! \brief Prepares the object for use by the application.
    //! \param device [in] - The device the pipeline belongs to.
    //! \returns True if the pipeline initialized successfully otherwise false.
    bool Pipeline::initialize(Device &device) {
        if (m_instance != VK_NULL_HANDLE) {
            printf("Pipeline already initialized\n");
            return false;
        }

        VkPipelineLayoutCreateInfo createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = 0;
        createInfo.pSetLayouts = nullptr;
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = nullptr;

        const auto result = vkCreatePipelineLayout(device, &createInfo, nullptr, &m_instance);
        if (result != VK_SUCCESS) {
            printf("Failed to create pipeline: %s\n", getResultString(result));
            return false;
        }

        m_device = device;
        return true;
    }
}
