

#include "semaphore.h"
#include "device.h"

namespace ngen::vulkan {
    Semaphore::Semaphore()
    : m_handle(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {
    }

    Semaphore::~Semaphore() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void Semaphore::dispose() {
        if (m_handle != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_device, m_handle, nullptr);
            m_handle = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    bool Semaphore::create(Device &device) {
        if (m_handle != VK_NULL_HANDLE) {
            return false;
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (VK_SUCCESS != vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_handle)) {
            return false;
        }

        m_device = device;
        return true;
    }
}
