// Spec: rhi-vulkan. Behaviour changes go through it.
#include "device_impl.h"

#include "surface.h"
#include "vulkan_result.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>

namespace ngen::rhi::vulkan
{
    namespace
    {
        constexpr uint32_t kSpirvMagic = 0x07230203;

        // Used for pipelines created while there is no swap chain.
        constexpr VkFormat kDefaultColorFormat = VK_FORMAT_B8G8R8A8_UNORM;

        std::unexpected<ngen::Error> invalid(std::string message)
        {
            return std::unexpected(makeError(ErrorCode::InvalidArgument, std::move(message)));
        }

        bool isZero(const ngen::platform::PixelSize size)
        {
            return size.width == 0 || size.height == 0;
        }

        template <typename T>
        void freeFinished(std::vector<Retired<T>> &retired, const uint64_t finishedThrough)
        {
            std::erase_if(retired, [finishedThrough](const Retired<T> &entry) {
                return entry.retiredAt <= finishedThrough;
            });
        }
    } // namespace

    std::expected<std::unique_ptr<DeviceImpl>, ngen::Error> DeviceImpl::create(const DeviceDescription &description,
                                                                               const WindowSupport windowSupport)
    {
        auto context = VulkanContext::create(description.applicationName, description.enableValidation,
                                             windowSupport == WindowSupport::Expected, description.messageSink);
        if (!context)
        {
            return std::unexpected(std::move(context.error()));
        }

        std::unique_ptr<DeviceImpl> device(new DeviceImpl(std::move(*context)));
        if (auto created = device->createFrameSlots(); !created)
        {
            return std::unexpected(std::move(created.error()));
        }
        return device;
    }

    DeviceImpl::DeviceImpl(std::unique_ptr<VulkanContext> context)
        : context_(std::move(context)),
          renderPasses_(std::make_unique<RenderPassCache>(
              context_->device(), context_->windowExpected() ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                                             : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)),
          commandList_(std::make_unique<VulkanCommandList>(*this))
    {
    }

    DeviceImpl::~DeviceImpl()
    {
        (void)waitForGpu();

        // Reverse order of creation: resources and the swap chain were made after the frame slots, and render
        // passes on first use after them.
        commandList_.reset();
        retiredPipelines_.clear();
        pipelines_ = {};
        shaders_ = {};
        retiredBuffers_.clear();
        buffers_ = {};
        destroySwapChainAndSurface();
        renderPasses_.reset();
        destroyFrameSlots();
        context_.reset();
    }

    std::expected<void, ngen::Error> DeviceImpl::createFrameSlots()
    {
        const VkDevice device = context_->device();
        for (FrameSlot &slot : slots_)
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = context_->queueFamily();
            if (const VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &slot.commandPool);
                result != VK_SUCCESS)
            {
                slot.commandPool = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateCommandPool", result));
            }

            VkCommandBufferAllocateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            bufferInfo.commandPool = slot.commandPool;
            bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            bufferInfo.commandBufferCount = 1;
            if (const VkResult result = vkAllocateCommandBuffers(device, &bufferInfo, &slot.commandBuffer);
                result != VK_SUCCESS)
            {
                slot.commandBuffer = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkAllocateCommandBuffers", result));
            }

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if (const VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &slot.imageAvailable);
                result != VK_SUCCESS)
            {
                slot.imageAvailable = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateSemaphore", result));
            }

            // Signalled, so the first wait on each slot returns at once.
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            if (const VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &slot.inFlight);
                result != VK_SUCCESS)
            {
                slot.inFlight = VK_NULL_HANDLE;
                return std::unexpected(vulkanError("vkCreateFence", result));
            }
        }
        return {};
    }

    void DeviceImpl::destroyFrameSlots()
    {
        const VkDevice device = context_->device();
        for (auto slot = slots_.rbegin(); slot != slots_.rend(); ++slot)
        {
            if (slot->inFlight != VK_NULL_HANDLE)
            {
                vkDestroyFence(device, slot->inFlight, nullptr);
            }
            if (slot->imageAvailable != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(device, slot->imageAvailable, nullptr);
            }
            // Destroying the pool frees its command buffer.
            if (slot->commandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(device, slot->commandPool, nullptr);
            }
            *slot = FrameSlot{};
        }
    }

    void DeviceImpl::report(const Severity severity, const std::string_view message) const
    {
        context_->report(severity, message);
    }

    const VulkanContext &DeviceImpl::context() const
    {
        return *context_;
    }

    RenderPassCache &DeviceImpl::renderPasses()
    {
        return *renderPasses_;
    }

    Buffer *DeviceImpl::findBuffer(const BufferHandle buffer)
    {
        return buffers_.get(buffer);
    }

    Pipeline *DeviceImpl::findPipeline(const PipelineHandle pipeline)
    {
        return pipelines_.get(pipeline);
    }

    std::expected<BufferHandle, ngen::Error> DeviceImpl::createBuffer(const BufferDescription &description)
    {
        if (auto valid = validate(description); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }
        auto buffer = Buffer::create(context_->allocator(), description);
        if (!buffer)
        {
            return std::unexpected(std::move(buffer.error()));
        }
        return buffers_.insert(std::move(*buffer));
    }

    std::expected<void, ngen::Error> DeviceImpl::writeBuffer(const BufferHandle buffer, const uint64_t offset,
                                                             const std::span<const std::byte> data)
    {
        Buffer *target = buffers_.get(buffer);
        if (target == nullptr)
        {
            return invalid("writeBuffer: the buffer handle is null or stale");
        }
        return target->write(offset, data);
    }

    std::expected<ShaderHandle, ngen::Error> DeviceImpl::createShader(const ShaderDescription &description)
    {
        if (auto valid = validate(description); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }

        const std::span<const std::byte> bytecode = description.bytecode;
        if (bytecode.size() % sizeof(uint32_t) != 0)
        {
            return invalid("ShaderDescription.bytecode is not SPIR-V: its size is not a multiple of 4 bytes");
        }
        std::vector<uint32_t> code(bytecode.size() / sizeof(uint32_t));
        std::memcpy(code.data(), bytecode.data(), bytecode.size());
        if (code.front() != kSpirvMagic)
        {
            return invalid("ShaderDescription.bytecode is not SPIR-V: it does not start with the SPIR-V magic number");
        }
        return shaders_.insert(VulkanShader{std::move(code)});
    }

    std::expected<PipelineHandle, ngen::Error> DeviceImpl::createPipeline(const GraphicsPipelineDescription &description)
    {
        if (auto valid = validate(description); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }
        const VulkanShader *vertex = shaders_.get(description.vertex.shader);
        if (vertex == nullptr)
        {
            return invalid("GraphicsPipelineDescription.vertex.shader is a null or stale handle");
        }
        const VulkanShader *fragment = shaders_.get(description.fragment.shader);
        if (fragment == nullptr)
        {
            return invalid("GraphicsPipelineDescription.fragment.shader is a null or stale handle");
        }
        if (description.polygonMode == PolygonMode::Line && !context_->fillModeNonSolid())
        {
            return std::unexpected(makeError(ErrorCode::Unsupported,
                                             "GraphicsPipelineDescription.polygonMode Line needs the "
                                             "fillModeNonSolid feature, which this device lacks"));
        }

        const VkFormat format = swapChain_ != nullptr ? swapChain_->format() : kDefaultColorFormat;
        auto pipeline = Pipeline::create(context_->device(), *renderPasses_, description,
                                         PipelineStage{vertex->code, description.vertex.entryPoint},
                                         PipelineStage{fragment->code, description.fragment.entryPoint}, format);
        if (!pipeline)
        {
            return std::unexpected(std::move(pipeline.error()));
        }
        return pipelines_.insert(std::move(*pipeline));
    }

    void DeviceImpl::destroy(const BufferHandle buffer)
    {
        if (auto removed = buffers_.remove(buffer); removed.has_value())
        {
            retiredBuffers_.push_back(Retired<Buffer>{std::move(*removed), framesBegun_});
        }
    }

    void DeviceImpl::destroy(const ShaderHandle shader)
    {
        // A shader is only bytecode, which no frame uses (pipelines keep their own copy), so it goes at once.
        (void)shaders_.remove(shader);
    }

    void DeviceImpl::destroy(const PipelineHandle pipeline)
    {
        if (auto removed = pipelines_.remove(pipeline); removed.has_value())
        {
            retiredPipelines_.push_back(Retired<Pipeline>{std::move(*removed), framesBegun_});
        }
    }

    std::expected<void, ngen::Error> DeviceImpl::waitForGpu()
    {
        if (const VkResult result = vkDeviceWaitIdle(context_->device()); result != VK_SUCCESS)
        {
            if (result == VK_ERROR_DEVICE_LOST)
            {
                deviceLost_ = true;
            }
            return std::unexpected(vulkanError("vkDeviceWaitIdle", result));
        }
        for (FrameSlot &slot : slots_)
        {
            slot.executing = false;
        }
        return {};
    }

    void DeviceImpl::freeFinishedResources()
    {
        // Frames finish in submission order on the one queue; the oldest still executing bounds what is done.
        uint64_t finishedThrough = framesBegun_;
        for (FrameSlot &slot : slots_)
        {
            if (!slot.executing)
            {
                continue;
            }
            const VkResult status = vkGetFenceStatus(context_->device(), slot.inFlight);
            if (status == VK_SUCCESS)
            {
                slot.executing = false;
                continue;
            }
            if (status == VK_ERROR_DEVICE_LOST)
            {
                deviceLost_ = true;
            }
            finishedThrough = std::min(finishedThrough, slot.frameNumber - 1);
        }
        freeFinished(retiredPipelines_, finishedThrough);
        freeFinished(retiredBuffers_, finishedThrough);
    }

    std::expected<VkSurfaceKHR, ngen::Error> DeviceImpl::createPresentableSurface(
        const ngen::platform::NativeWindow &window)
    {
        auto surface = createSurface(context_->instance(), window);
        if (!surface)
        {
            return surface;
        }

        VkBool32 supported = VK_FALSE;
        const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(context_->physicalDevice(),
                                                                     context_->queueFamily(), *surface, &supported);
        if (result != VK_SUCCESS || supported != VK_TRUE)
        {
            vkDestroySurfaceKHR(context_->instance(), *surface, nullptr);
            if (result != VK_SUCCESS)
            {
                return std::unexpected(vulkanError("vkGetPhysicalDeviceSurfaceSupportKHR", result));
            }
            return std::unexpected(makeError(ErrorCode::Unsupported,
                                             "the device's queue family cannot present to this window's surface"));
        }
        return surface;
    }

    std::expected<void, ngen::Error> DeviceImpl::recreateSwapChain()
    {
        if (auto waited = waitForGpu(); !waited)
        {
            return waited;
        }
        recreatePending_ = false;
        if (isZero(windowSize_))
        {
            swapChain_.reset();
            return {};
        }

        // The old swap chain is retired by the call whether or not it succeeds, so it goes either way.
        auto next = SwapChain::create(*context_, *renderPasses_, surface_, windowSize_, swapChain_.get());
        swapChain_.reset();
        if (!next)
        {
            return std::unexpected(std::move(next.error()));
        }
        swapChain_ = std::move(*next);
        return {};
    }

    void DeviceImpl::destroySwapChainAndSurface()
    {
        swapChain_.reset();
        if (surface_ != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(context_->instance(), surface_, nullptr);
            surface_ = VK_NULL_HANDLE;
        }
        recreatePending_ = false;
    }

    std::unexpected<ngen::Error> DeviceImpl::frameFailure(ngen::Error error)
    {
        if (error.code == ErrorCode::DeviceLost)
        {
            deviceLost_ = true;
        }
        else if (error.code == ErrorCode::SurfaceLost)
        {
            // Frames in flight finish before their swap chain goes.
            (void)waitForGpu();
            destroySwapChainAndSurface();
            surfaceLost_ = true;
        }
        return std::unexpected(std::move(error));
    }

    std::expected<void, ngen::Error> DeviceImpl::attachWindow(const ngen::platform::NativeWindow &window,
                                                              const ngen::platform::PixelSize size)
    {
        if (attached_)
        {
            return invalid("attachWindow: the device already has a swap chain");
        }
        if (!context_->windowExpected())
        {
            return std::unexpected(makeError(ErrorCode::Unsupported,
                                             "attachWindow: the device was created with WindowSupport::None"));
        }

        auto surface = createPresentableSurface(window);
        if (!surface)
        {
            return std::unexpected(std::move(surface.error()));
        }
        surface_ = *surface;
        windowSize_ = size;
        if (auto created = recreateSwapChain(); !created)
        {
            destroySwapChainAndSurface();
            return created;
        }
        attached_ = true;
        suspended_ = false;
        surfaceLost_ = false;
        return {};
    }

    void DeviceImpl::onResized(const ngen::platform::PixelSize size)
    {
        if (!attached_)
        {
            return;
        }
        windowSize_ = size;
        // With no swap chain, beginFrame makes one for the first non-zero size.
        if (swapChain_ != nullptr)
        {
            const VkExtent2D extent = swapChain_->extent();
            if (size.width != extent.width || size.height != extent.height)
            {
                recreatePending_ = true;
            }
        }
    }

    void DeviceImpl::suspend()
    {
        (void)waitForGpu();
        destroySwapChainAndSurface();
        suspended_ = true;
    }

    void DeviceImpl::onSuspended()
    {
        if (!attached_)
        {
            return;
        }
        if (recording_)
        {
            // The frame being recorded still uses the swap chain; endFrame suspends once it is submitted.
            suspendAfterFrame_ = true;
            return;
        }
        suspend();
    }

    std::expected<void, ngen::Error> DeviceImpl::onResumed(const ngen::platform::NativeWindow &window,
                                                           const ngen::platform::PixelSize size)
    {
        if (!attached_)
        {
            return invalid("onResumed: no window is attached");
        }
        if (recording_)
        {
            report(Severity::Error, "onResumed: called while a frame is being recorded");
            return invalid("onResumed: called while a frame is being recorded");
        }

        // Handles from before a suspension are invalid, so whatever surface remains is replaced.
        suspend();

        auto surface = createPresentableSurface(window);
        if (!surface)
        {
            return std::unexpected(std::move(surface.error()));
        }
        surface_ = *surface;
        windowSize_ = size;
        if (auto created = recreateSwapChain(); !created)
        {
            destroySwapChainAndSurface();
            return created;
        }
        suspended_ = false;
        surfaceLost_ = false;
        return {};
    }

    std::expected<CommandList *, ngen::Error> DeviceImpl::beginFrame()
    {
        if (recording_)
        {
            report(Severity::Error, "beginFrame: a frame is already being recorded");
            return invalid("beginFrame: a frame is already being recorded");
        }

        freeFinishedResources();

        if (deviceLost_)
        {
            return std::unexpected(makeError(ErrorCode::DeviceLost, "beginFrame: the device was lost"));
        }
        if (!attached_ || suspended_ || surfaceLost_ || surface_ == VK_NULL_HANDLE)
        {
            return nullptr;
        }
        if (recreatePending_ || (swapChain_ == nullptr && !isZero(windowSize_)))
        {
            if (auto recreated = recreateSwapChain(); !recreated)
            {
                return frameFailure(std::move(recreated.error()));
            }
        }
        if (swapChain_ == nullptr)
        {
            return nullptr;
        }

        const VkDevice device = context_->device();
        FrameSlot &slot = slots_[slotIndex_];

        // The slot's command buffer and semaphore are reused only once its last frame has finished.
        if (const VkResult result = vkWaitForFences(device, 1, &slot.inFlight, VK_TRUE, UINT64_MAX);
            result != VK_SUCCESS)
        {
            return frameFailure(vulkanError("vkWaitForFences", result));
        }
        slot.executing = false;

        // An out-of-date swap chain is recreated and the acquire tried once more; a suboptimal one still
        // renders this frame.
        bool acquired = false;
        for (int attempt = 0; attempt < 2 && !acquired; ++attempt)
        {
            const VkResult result = vkAcquireNextImageKHR(device, swapChain_->handle(), UINT64_MAX,
                                                          slot.imageAvailable, VK_NULL_HANDLE, &imageIndex_);
            if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
            {
                acquired = true;
            }
            else if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                if (auto recreated = recreateSwapChain(); !recreated)
                {
                    return frameFailure(std::move(recreated.error()));
                }
                if (swapChain_ == nullptr)
                {
                    return nullptr;
                }
            }
            else
            {
                return frameFailure(vulkanError("vkAcquireNextImageKHR", result));
            }
        }
        if (!acquired)
        {
            recreatePending_ = true;
            return nullptr;
        }

        // An earlier frame from the other slot may still be rendering to this image.
        SwapChainImage &image = swapChain_->images()[imageIndex_];
        if (image.inFlight != VK_NULL_HANDLE && image.inFlight != slot.inFlight)
        {
            if (const VkResult result = vkWaitForFences(device, 1, &image.inFlight, VK_TRUE, UINT64_MAX);
                result != VK_SUCCESS)
            {
                return abandonAcquiredImage(slot, vulkanError("vkWaitForFences", result));
            }
        }
        image.inFlight = slot.inFlight;

        if (const VkResult result = vkResetCommandPool(device, slot.commandPool, 0); result != VK_SUCCESS)
        {
            return abandonAcquiredImage(slot, vulkanError("vkResetCommandPool", result));
        }
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (const VkResult result = vkBeginCommandBuffer(slot.commandBuffer, &beginInfo); result != VK_SUCCESS)
        {
            return abandonAcquiredImage(slot, vulkanError("vkBeginCommandBuffer", result));
        }

        ++framesBegun_;
        slot.frameNumber = framesBegun_;
        recording_ = true;
        commandList_->begin(FrameTarget{slot.commandBuffer, &image, swapChain_->format(), swapChain_->extent()});
        return commandList_.get();
    }

    std::expected<void, ngen::Error> DeviceImpl::endFrame()
    {
        if (!recording_)
        {
            report(Severity::Error, "endFrame: no frame is being recorded");
            return invalid("endFrame: no frame is being recorded");
        }
        recording_ = false;

        FrameSlot &slot = slots_[slotIndex_];
        SwapChainImage &image = swapChain_->images()[imageIndex_];
        commandList_->finish();
        auto outcome = submitAndPresent(slot, image);

        slotIndex_ = (slotIndex_ + 1) % kMaxFramesInFlight;
        if (suspendAfterFrame_)
        {
            suspendAfterFrame_ = false;
            suspend();
        }
        return outcome;
    }

    std::expected<void, ngen::Error> DeviceImpl::submitAndPresent(FrameSlot &slot, SwapChainImage &image)
    {
        const VkDevice device = context_->device();
        if (const VkResult result = vkEndCommandBuffer(slot.commandBuffer); result != VK_SUCCESS)
        {
            return abandonAcquiredImage(slot, vulkanError("vkEndCommandBuffer", result));
        }

        const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &slot.imageAvailable;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &slot.commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &image.renderFinished;

        // Reset only now, immediately before the submit that signals it, so no wait can find it unsignalled
        // with nothing to signal it.
        if (const VkResult result = vkResetFences(device, 1, &slot.inFlight); result != VK_SUCCESS)
        {
            return abandonAcquiredImage(slot, vulkanError("vkResetFences", result));
        }
        if (const VkResult result = vkQueueSubmit(context_->queue(), 1, &submitInfo, slot.inFlight);
            result != VK_SUCCESS)
        {
            return abandonAcquiredImage(slot, vulkanError("vkQueueSubmit", result));
        }
        slot.executing = true;

        VkSwapchainKHR swapChain = swapChain_->handle();
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &image.renderFinished;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = &imageIndex_;

        const VkResult result = vkQueuePresentKHR(context_->queue(), &presentInfo);
        if (result == VK_SUCCESS)
        {
            return {};
        }
        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreatePending_ = true;
            return {};
        }
        return frameFailure(vulkanError("vkQueuePresentKHR", result));
    }

    std::unexpected<ngen::Error> DeviceImpl::abandonAcquiredImage(FrameSlot &slot, ngen::Error error)
    {
        if (error.code != ErrorCode::DeviceLost)
        {
            // The acquired image is never presented, so only a new swap chain releases it.
            recreatePending_ = true;

            // Nothing waits on the slot's semaphore, which the acquire signalled, and nothing will signal its
            // fence; an empty batch does both, so the slot can be used again. The fence is reset immediately
            // before that submit if it is still signalled (the failure came before the frame's own reset).
            bool fenceReady = true;
            if (vkGetFenceStatus(context_->device(), slot.inFlight) == VK_SUCCESS)
            {
                fenceReady = vkResetFences(context_->device(), 1, &slot.inFlight) == VK_SUCCESS;
            }
            const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &slot.imageAvailable;
            submitInfo.pWaitDstStageMask = &waitStage;
            if (fenceReady && vkQueueSubmit(context_->queue(), 1, &submitInfo, slot.inFlight) == VK_SUCCESS)
            {
                slot.executing = true;
            }
            else
            {
                // The queue accepts no work, so the semaphore stays signalled and the fence may never signal;
                // the device is treated as lost rather than let the next beginFrame wait forever.
                deviceLost_ = true;
            }
        }
        return frameFailure(std::move(error));
    }

    ngen::platform::PixelSize DeviceImpl::swapChainSize() const
    {
        if (swapChain_ == nullptr)
        {
            return ngen::platform::PixelSize{0, 0};
        }
        const VkExtent2D extent = swapChain_->extent();
        return ngen::platform::PixelSize{extent.width, extent.height};
    }

    void DeviceImpl::waitIdle()
    {
        (void)waitForGpu();
    }
} // namespace ngen::rhi::vulkan
