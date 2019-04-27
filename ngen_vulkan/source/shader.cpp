

#include "shader.h"
#include "vulkan_error.h"
#include "device.h"

namespace ngen::vulkan {
    Shader::Shader()
    : m_handle(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {
    }

    Shader::~Shader() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void Shader::dispose() {
        if (m_handle != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, m_handle, nullptr);

            m_device = VK_NULL_HANDLE;
            m_handle = VK_NULL_HANDLE;
        }
    }

    //! \brief Attempts to create a shader object using the supplied bytecode.
    //! \param device [in] - The device the shader should be created with.
    //! \param code [in] - Pointer to the raw bytecode for the shader.
    //! \param length [in] - The length (in bytes) of the supplied bytecode data.
    //! \returns <em>True</em> if the shader created successfully otherwise <em>false</en>.
    bool Shader::create(Device &device, void *code, size_t length) {
        VkShaderModuleCreateInfo info = {};

        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = length;
        info.pCode = static_cast<uint32_t*>(code);

        VkResult result = vkCreateShaderModule(device, &info, nullptr, &m_handle);
        if (result != VK_SUCCESS) {
            printf("Failed to create shader module: %s.\n", getResultString(result));
            return false;
        }

        m_device = device;
        return true;
    }
}
