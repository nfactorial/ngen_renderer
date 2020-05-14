
#ifndef NGEN_VULKAN_SEMAPHORE
#define NGEN_VULKAN_SEMAPHORE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;

    class Semaphore {
    public:
        Semaphore();
        ~Semaphore();

        Semaphore(const Semaphore &other) = delete;
        Semaphore& operator=(const Semaphore &other) = delete;

        void dispose();
        [[nodiscard]] bool create(Device &device);

        operator VkSemaphore() const;  // NOLINT

    private:
        VkSemaphore m_handle;
        VkDevice m_device;
    };

    inline Semaphore::operator VkSemaphore() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_SEMAPHORE
