// Spec: rhi-vulkan. Behaviour changes go through it.
#include "swap_chain.h"

#include "vulkan_result.h"

#include <algorithm>
#include <array>
#include <limits>

namespace ngen::rhi::vulkan
{
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::span<const VkSurfaceFormatKHR> formats)
    {
        for (const VkSurfaceFormatKHR &format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }
        return formats.front();
    }

    VkPresentModeKHR choosePresentMode(const std::span<const VkPresentModeKHR> modes)
    {
        if (modes.empty() || std::ranges::find(modes, VK_PRESENT_MODE_FIFO_KHR) != modes.end())
        {
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        return modes.front();
    }

    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, const ngen::platform::PixelSize windowSize)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        return VkExtent2D{
            std::clamp(windowSize.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(windowSize.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
    }

    uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        const uint32_t count = capabilities.minImageCount + 1;
        return capabilities.maxImageCount > 0 ? std::min(count, capabilities.maxImageCount) : count;
    }

    VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkCompositeAlphaFlagsKHR supported)
    {
        constexpr std::array<VkCompositeAlphaFlagBitsKHR, 4> preferred = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};
        for (const VkCompositeAlphaFlagBitsKHR mode : preferred)
        {
            if ((supported & mode) != 0)
            {
                return mode;
            }
        }
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    std::expected<std::unique_ptr<SwapChain>, ngen::Error> SwapChain::create(const VulkanContext &context,
                                                                             RenderPassCache &renderPasses,
                                                                             const VkSurfaceKHR surface,
                                                                             const ngen::platform::PixelSize windowSize,
                                                                             const SwapChain *const old)
    {
        const VkPhysicalDevice physicalDevice = context.physicalDevice();

        VkSurfaceCapabilitiesKHR capabilities{};
        if (const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
            result != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", result));
        }

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        if (const VkResult result =
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
            result != VK_SUCCESS && result != VK_INCOMPLETE)
        {
            return std::unexpected(vulkanError("vkGetPhysicalDeviceSurfaceFormatsKHR", result));
        }
        formats.resize(formatCount);
        if (formats.empty())
        {
            return std::unexpected(makeError(ErrorCode::Unsupported, "the surface offers no formats"));
        }

        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, nullptr);
        std::vector<VkPresentModeKHR> modes(modeCount);
        if (const VkResult result =
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, modes.data());
            result != VK_SUCCESS && result != VK_INCOMPLETE)
        {
            return std::unexpected(vulkanError("vkGetPhysicalDeviceSurfacePresentModesKHR", result));
        }
        modes.resize(modeCount);

        const VkExtent2D extent = chooseExtent(capabilities, windowSize);
        if (extent.width == 0 || extent.height == 0)
        {
            return std::unique_ptr<SwapChain>();
        }
        const VkSurfaceFormatKHR format = chooseSurfaceFormat(formats);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = chooseImageCount(capabilities);
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // one queue family renders and presents
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = chooseCompositeAlpha(capabilities.supportedCompositeAlpha);
        createInfo.presentMode = choosePresentMode(modes);
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = old != nullptr ? old->handle() : VK_NULL_HANDLE;

        const VkDevice device = context.device();
        std::unique_ptr<SwapChain> swapChain(new SwapChain(device));
        if (const VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain->swapChain_);
            result != VK_SUCCESS)
        {
            swapChain->swapChain_ = VK_NULL_HANDLE;
            return std::unexpected(vulkanError("vkCreateSwapchainKHR", result));
        }
        swapChain->format_ = format.format;
        swapChain->extent_ = extent;

        const auto renderPass = renderPasses.compatible(format.format);
        if (!renderPass)
        {
            return std::unexpected(renderPass.error());
        }

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(device, swapChain->swapChain_, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        if (const VkResult result = vkGetSwapchainImagesKHR(device, swapChain->swapChain_, &imageCount, images.data());
            result != VK_SUCCESS)
        {
            return std::unexpected(vulkanError("vkGetSwapchainImagesKHR", result));
        }

        for (const VkImage image : images)
        {
            // Pushed first so the destructor frees whatever part of it was made.
            SwapChainImage &entry = swapChain->images_.emplace_back();
            entry.image = image;

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format.format;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;
            if (const VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &entry.view); result != VK_SUCCESS)
            {
                entry.view = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateImageView", result));
            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &entry.view;
            framebufferInfo.width = extent.width;
            framebufferInfo.height = extent.height;
            framebufferInfo.layers = 1;
            if (const VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &entry.framebuffer);
                result != VK_SUCCESS)
            {
                entry.framebuffer = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateFramebuffer", result));
            }

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if (const VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &entry.renderFinished);
                result != VK_SUCCESS)
            {
                entry.renderFinished = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateSemaphore", result));
            }
        }
        return swapChain;
    }

    SwapChain::SwapChain(const VkDevice device) : device_(device)
    {
    }

    SwapChain::~SwapChain()
    {
        for (auto image = images_.rbegin(); image != images_.rend(); ++image)
        {
            if (image->renderFinished != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(device_, image->renderFinished, nullptr);
            }
            if (image->framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device_, image->framebuffer, nullptr);
            }
            if (image->view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device_, image->view, nullptr);
            }
        }
        if (swapChain_ != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device_, swapChain_, nullptr);
        }
    }
} // namespace ngen::rhi::vulkan
