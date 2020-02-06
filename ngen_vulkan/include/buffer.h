
#ifndef NGEN_VULKAN_BUFFER
#define NGEN_VULKAN_BUFFER

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class Buffer {
    public:
        Buffer();
        ~Buffer();

        void dispose();
        bool create(Device &device, void *code, size_t length);

        operator VkShaderModule() const;  // NOLINT

    private:
        VkShaderModule m_handle;
        VkDevice m_device;
    };

    inline Shader::operator VkShaderModule() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_BUFFER
