

#include "fence.h"
#include "device.h"

namespace ngen::vulkan {
    Fence::Fence()
    : m_handle(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE) {
    }

    Fence::~Fence() {
        dispose();
    }

    //! \brief Releases all resources currently referenced by this object.
    void Fence::dispose() {
        if (m_handle != VK_NULL_HANDLE) {
            vkDestroyFence(m_device, m_handle, nullptr);
            m_handle = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    //! \brief Attempts to create a new Fence object.
    //! \param device [in] - The device to use when creating the fence.
    //! \param signaled [in] - If <em>true</em> the fence will be created in a signaled state otherwise it will be unsignaled.
    //! \returns <em>True</em> if the fence created successfully otherwise <em>false</em>.
    bool Fence::create(Device &device, bool signaled) {
        if (m_handle != VK_NULL_HANDLE) {
            return false;
        }

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        if (VK_SUCCESS != vkCreateFence(device, &fenceInfo, nullptr, &m_handle)) {
            return false;
        }

        m_device = device;
        return true;
    }

    //! \brief Waits an indeterminate time for the fence to become signaled.
    void Fence::wait() {
        return wait(UINT64_MAX);
    }

    //! \brief Waits for the fence to become signaled.
    //! \param timeout [in] - Timeout (in ms) to wait before giving up.
    void Fence::wait(uint64_t timeout) {
        if (m_handle != VK_NULL_HANDLE) {
            vkWaitForFences(m_device, 1, &m_handle, VK_TRUE, timeout);
        }
    }

    //! \brief Resets the fence to the unsignaled state.
    void Fence::reset() {
        if (m_handle != VK_NULL_HANDLE) {
            vkResetFences(m_device, 1, &m_handle);
        }
    }
}
