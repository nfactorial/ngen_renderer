
#ifndef NGEN_VULKAN_FENCE
#define NGEN_VULKAN_FENCE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;

    class Fence {
    public:
        Fence();
        ~Fence();

        Fence(const Fence &other) = delete;
        Fence& operator=(const Fence &other) = delete;

        void dispose();
        void reset();

        void wait();
        void wait(uint64_t timeout);

        [[nodiscard]] bool create(Device &device, bool signaled = false);

        operator VkFence() const;  // NOLINT

    private:
        VkFence m_handle;
        VkDevice m_device;
    };

    inline Fence::operator VkFence() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_FENCE
