
#ifndef NGEN_VULKAN_PIPELINE
#define NGEN_VULKAN_PIPELINE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class Device;

    class Pipeline {
    public:
        Pipeline();
        ~Pipeline();

        void dispose();
        [[nodiscard]] bool initialize(Device &device);

        operator VkPipelineLayout () const;

    private:
        VkPipelineLayout m_instance;
        VkDevice m_device;
    };

    //! \brief Conversion operator for VKPipelineLayout
    inline Pipeline::operator VkPipelineLayout() const {
        return m_instance;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_PIPELINE
