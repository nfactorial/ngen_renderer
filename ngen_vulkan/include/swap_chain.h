
#ifndef NGEN_VULKAN_SWAP_CHAIN
#define NGEN_VULKAN_SWAP_CHAIN

////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class PhysicalDevice;
    class WindowSurface;
    class VulkanContext;
    class RenderPass;
    class Device;

    class SwapChain {
    public:
        SwapChain();
        ~SwapChain();

        SwapChain(const SwapChain &other) = delete;
        SwapChain& operator=(const SwapChain &other) = delete;

        void dispose();

        void initialize(const PhysicalDevice &physicalDevice, const WindowSurface &surface);
        [[nodiscard]] bool create(Device &device, WindowSurface &surface);

        operator VkSwapchainKHR () const; // NOLINT

        [[nodiscard]] VkFormat getImageFormat() const;
        [[nodiscard]] bool isUsable() const;

        [[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat() const;
        [[nodiscard]] VkPresentModeKHR choosePresentMode() const;

        [[nodiscard]] bool acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t *imageIndex);
        [[nodiscard]] bool acquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t *imageIndex);

        [[nodiscard]] size_t getImageCount() const;

        [[nodiscard]] const VkExtent2D& getExtent() const;

        [[nodiscard]] bool createFrameBuffers(const RenderPass &renderPass, std::vector<VkFramebuffer> &frameBuffers) const;

    private:
        [[nodiscard]] const VkExtent2D& chooseExtent(int width, int height);

        void enumerateSurfaceFormats(const PhysicalDevice &physicalDevice, const WindowSurface &surface);
        void enumeratePresentModes(const PhysicalDevice &physicalDevice, const WindowSurface &surface);

        [[nodiscard]] bool createImageViews();
        void extractImages();

        void destroyImageViews();

    private:
        std::vector<VkPresentModeKHR> m_presentModes;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkImageView> m_imageViews;
        std::vector<VkImage> m_images;

        VkSurfaceCapabilitiesKHR m_capabilities;
        VkFormat m_imageFormat;

        VkSwapchainKHR m_handle;
        VkExtent2D m_extent;
        VkDevice m_device;
    };

    inline const VkExtent2D& SwapChain::getExtent() const {
        return m_extent;
    }

    inline VkFormat SwapChain::getImageFormat() const {
        return m_imageFormat;
    }

    inline size_t SwapChain::getImageCount() const {
        return m_images.size();
    }

    inline SwapChain::operator VkSwapchainKHR () const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_SWAP_CHAIN
