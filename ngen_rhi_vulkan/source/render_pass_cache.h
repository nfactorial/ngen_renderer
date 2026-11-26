// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/rhi/descriptions.h>

#include <vulkan/vulkan.h>

#include <expected>
#include <vector>

namespace ngen::rhi::vulkan
{
    // One single-subpass VkRenderPass per distinct (colour format, LoadOp, StoreOp), created on first use and
    // kept until the cache is destroyed. Every pass of a format is compatible with every other, so pipelines
    // and framebuffers made against one work with all of them.
    class RenderPassCache
    {
    public:
        // targetLayout is the layout the colour attachment ends in (and, for LoadOp::Load, starts in):
        // PRESENT_SRC_KHR for a device that presents, COLOR_ATTACHMENT_OPTIMAL for one that never does
        // (PRESENT_SRC_KHR needs VK_KHR_swapchain).
        RenderPassCache(VkDevice device, VkImageLayout targetLayout);
        ~RenderPassCache();
        RenderPassCache(const RenderPassCache &) = delete;
        RenderPassCache &operator=(const RenderPassCache &) = delete;

        [[nodiscard]] std::expected<VkRenderPass, ngen::Error> get(VkFormat format, LoadOp load, StoreOp store);

        // The pass pipelines and framebuffers are made against: load Clear, store Store.
        [[nodiscard]] std::expected<VkRenderPass, ngen::Error> compatible(VkFormat format)
        {
            return get(format, LoadOp::Clear, StoreOp::Store);
        }

    private:
        struct Entry
        {
            VkFormat format;
            LoadOp load;
            StoreOp store;
            VkRenderPass renderPass;
        };

        VkDevice device_;
        VkImageLayout targetLayout_;
        std::vector<Entry> entries_;
    };
} // namespace ngen::rhi::vulkan
