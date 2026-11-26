// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include "buffer.h"
#include "command_list.h"
#include "pipeline.h"
#include "render_pass_cache.h"
#include "swap_chain.h"
#include "vulkan_context.h"

#include <ngen/error.h>
#include <ngen/platform/window.h>
#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/device.h>
#include <ngen/rhi/handle.h>
#include <ngen/rhi/vulkan/vulkan_device.h>

#include <vulkan/vulkan.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace ngen::rhi::vulkan
{
    // One of the kMaxFramesInFlight frame slots.
    struct FrameSlot
    {
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkSemaphore imageAvailable = VK_NULL_HANDLE;
        VkFence inFlight = VK_NULL_HANDLE; // signalled by the slot's last submit; created signalled
        uint64_t frameNumber = 0;          // the frame last begun in this slot
        bool executing = false;            // submitted, and its fence not yet seen signalled
    };

    // A destroyed resource, kept until every frame begun before its destroy call has finished.
    template <typename T>
    struct Retired
    {
        T resource;
        uint64_t retiredAt; // the number of frames begun when it was destroyed
    };

    // Everything VulkanDevice does. VulkanDevice forwards to it so the public header names no Vulkan type.
    class DeviceImpl
    {
    public:
        [[nodiscard]] static std::expected<std::unique_ptr<DeviceImpl>, ngen::Error> create(
            const DeviceDescription &description, WindowSupport windowSupport);

        ~DeviceImpl();
        DeviceImpl(const DeviceImpl &) = delete;
        DeviceImpl &operator=(const DeviceImpl &) = delete;

        [[nodiscard]] std::expected<BufferHandle, ngen::Error> createBuffer(const BufferDescription &description);
        [[nodiscard]] std::expected<void, ngen::Error> writeBuffer(BufferHandle buffer, uint64_t offset,
                                                                   std::span<const std::byte> data);
        [[nodiscard]] std::expected<ShaderHandle, ngen::Error> createShader(const ShaderDescription &description);
        [[nodiscard]] std::expected<PipelineHandle, ngen::Error> createPipeline(
            const GraphicsPipelineDescription &description);

        void destroy(BufferHandle buffer);
        void destroy(ShaderHandle shader);
        void destroy(PipelineHandle pipeline);

        [[nodiscard]] std::expected<void, ngen::Error> attachWindow(const ngen::platform::NativeWindow &window,
                                                                    ngen::platform::PixelSize size);
        void onResized(ngen::platform::PixelSize size);
        void onSuspended();
        [[nodiscard]] std::expected<void, ngen::Error> onResumed(const ngen::platform::NativeWindow &window,
                                                                 ngen::platform::PixelSize size);

        [[nodiscard]] std::expected<CommandList *, ngen::Error> beginFrame();
        [[nodiscard]] std::expected<void, ngen::Error> endFrame();
        [[nodiscard]] ngen::platform::PixelSize swapChainSize() const;
        void waitIdle();

        // For the command list.
        void report(Severity severity, std::string_view message) const;
        [[nodiscard]] const VulkanContext &context() const;
        [[nodiscard]] RenderPassCache &renderPasses();
        [[nodiscard]] Buffer *findBuffer(BufferHandle buffer);
        [[nodiscard]] Pipeline *findPipeline(PipelineHandle pipeline);

    private:
        explicit DeviceImpl(std::unique_ptr<VulkanContext> context);

        [[nodiscard]] std::expected<void, ngen::Error> createFrameSlots();
        void destroyFrameSlots();

        // Waits for every submitted frame; DeviceLost if the device was lost.
        [[nodiscard]] std::expected<void, ngen::Error> waitForGpu();

        // Marks slots whose fence has signalled as finished, then frees retired resources no frame in flight
        // can still use.
        void freeFinishedResources();

        // A surface for the window, which the device's queue family must be able to present to.
        [[nodiscard]] std::expected<VkSurfaceKHR, ngen::Error> createPresentableSurface(
            const ngen::platform::NativeWindow &window);

        // After the GPU finishes its frames, replaces the swap chain with one matching the surface and window
        // size (none while either dimension is 0).
        [[nodiscard]] std::expected<void, ngen::Error> recreateSwapChain();
        void destroySwapChainAndSurface();

        // Records the consequence of a frame-path error (device loss; surface loss destroys the swap chain and
        // surface) and returns it.
        [[nodiscard]] std::unexpected<ngen::Error> frameFailure(ngen::Error error);

        void suspend();

        // endFrame's work once the command list has finished: submit, then present.
        [[nodiscard]] std::expected<void, ngen::Error> submitAndPresent(FrameSlot &slot, SwapChainImage &image);

        // For a failure after a successful acquire: unless the device was lost, flags recreation (releasing the
        // acquired image) and submits an empty batch waiting on the slot's semaphore and signalling its fence,
        // so the slot can be used again. Then records the failure as frameFailure does.
        [[nodiscard]] std::unexpected<ngen::Error> abandonAcquiredImage(FrameSlot &slot, ngen::Error error);

        std::unique_ptr<VulkanContext> context_;
        std::unique_ptr<RenderPassCache> renderPasses_;
        std::array<FrameSlot, kMaxFramesInFlight> slots_{};

        HandlePool<BufferTag, Buffer> buffers_;
        HandlePool<ShaderTag, VulkanShader> shaders_;
        HandlePool<PipelineTag, Pipeline> pipelines_;
        std::vector<Retired<Buffer>> retiredBuffers_;
        std::vector<Retired<Pipeline>> retiredPipelines_;

        VkSurfaceKHR surface_ = VK_NULL_HANDLE;
        std::unique_ptr<SwapChain> swapChain_;
        ngen::platform::PixelSize windowSize_{0, 0};
        bool attached_ = false;
        bool suspended_ = false;
        bool suspendAfterFrame_ = false; // onSuspended arrived while a frame was being recorded
        bool surfaceLost_ = false;
        bool deviceLost_ = false;
        bool recreatePending_ = false;

        std::unique_ptr<VulkanCommandList> commandList_;
        bool recording_ = false;
        uint32_t slotIndex_ = 0;
        uint32_t imageIndex_ = 0;
        uint64_t framesBegun_ = 0;
    };
} // namespace ngen::rhi::vulkan
