// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include "render_pass_cache.h"
#include "vulkan_context.h"

#include <ngen/error.h>
#include <ngen/platform/window.h>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <vector>

namespace ngen::rhi::vulkan
{
    // B8G8R8A8_UNORM with sRGB non-linear colour space if offered, else the first offered format.
    [[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat(std::span<const VkSurfaceFormatKHR> formats);

    // FIFO (always available, so frame pacing follows the display); the first offered mode only if a broken
    // driver leaves FIFO out.
    [[nodiscard]] VkPresentModeKHR choosePresentMode(std::span<const VkPresentModeKHR> modes);

    // The surface's current extent, or the window size clamped to the surface's limits when the surface lets
    // the swap chain decide.
    [[nodiscard]] VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                                          ngen::platform::PixelSize windowSize);

    // One more than the minimum, within the maximum (0 means none).
    [[nodiscard]] uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);

    // Opaque if supported, else the first supported mode.
    [[nodiscard]] VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(VkCompositeAlphaFlagsKHR supported);

    struct SwapChainImage
    {
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkSemaphore renderFinished = VK_NULL_HANDLE;
        VkFence inFlight = VK_NULL_HANDLE; // the fence of the frame slot that last rendered this image
        bool rendered = false;             // a frame has left it in the render passes' target layout
    };

    // A swap chain, its image views, one framebuffer and one render-finished semaphore per image. The format,
    // extent, image count and present mode are read from the surface each time one is made.
    class SwapChain
    {
    public:
        // Makes a swap chain for the surface at the window size, passing old (if any) as oldSwapchain; the
        // caller destroys old afterwards, whether or not this succeeds, and has waited for the GPU to finish
        // with it. An empty pointer (no error) when the surface's extent is 0 in either dimension.
        [[nodiscard]] static std::expected<std::unique_ptr<SwapChain>, ngen::Error> create(
            const VulkanContext &context, RenderPassCache &renderPasses, VkSurfaceKHR surface,
            ngen::platform::PixelSize windowSize, const SwapChain *old);

        ~SwapChain();
        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        [[nodiscard]] VkSwapchainKHR handle() const
        {
            return swapChain_;
        }

        [[nodiscard]] VkFormat format() const
        {
            return format_;
        }

        [[nodiscard]] VkExtent2D extent() const
        {
            return extent_;
        }

        [[nodiscard]] std::vector<SwapChainImage> &images()
        {
            return images_;
        }

    private:
        explicit SwapChain(VkDevice device);

        VkDevice device_;
        VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
        VkFormat format_ = VK_FORMAT_UNDEFINED;
        VkExtent2D extent_{0, 0};
        std::vector<SwapChainImage> images_;
    };
} // namespace ngen::rhi::vulkan
