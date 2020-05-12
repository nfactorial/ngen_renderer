
#ifndef NGEN_VULKAN_SHADER
#define NGEN_VULKAN_SHADER

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class Device;

    class Shader {
    public:
        Shader();
        ~Shader();

        Shader(const Shader &other) = delete;
        Shader& operator=(const Shader &other) = delete;

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

#endif //NGEN_VULKAN_SHADER
