// Spec: rhi-vulkan. Behaviour changes go through it.
#include <ngen/rhi/vulkan/vulkan_device.h>

#include "device_impl.h"

#include <utility>

namespace ngen::rhi::vulkan
{
    std::expected<std::unique_ptr<Device>, ngen::Error> VulkanDevice::create(const DeviceDescription &description,
                                                                            const WindowSupport windowSupport)
    {
        auto impl = DeviceImpl::create(description, windowSupport);
        if (!impl)
        {
            return std::unexpected(std::move(impl.error()));
        }
        return std::unique_ptr<Device>(new VulkanDevice(std::move(*impl)));
    }

    VulkanDevice::VulkanDevice(std::unique_ptr<DeviceImpl> impl) : impl_(std::move(impl))
    {
    }

    VulkanDevice::~VulkanDevice() = default;

    std::expected<BufferHandle, ngen::Error> VulkanDevice::createBuffer(const BufferDescription &description)
    {
        return impl_->createBuffer(description);
    }

    std::expected<void, ngen::Error> VulkanDevice::writeBuffer(const BufferHandle buffer, const uint64_t offset,
                                                               const std::span<const std::byte> data)
    {
        return impl_->writeBuffer(buffer, offset, data);
    }

    std::expected<ShaderHandle, ngen::Error> VulkanDevice::createShader(const ShaderDescription &description)
    {
        return impl_->createShader(description);
    }

    std::expected<PipelineHandle, ngen::Error> VulkanDevice::createPipeline(
        const GraphicsPipelineDescription &description)
    {
        return impl_->createPipeline(description);
    }

    void VulkanDevice::destroy(const BufferHandle buffer)
    {
        impl_->destroy(buffer);
    }

    void VulkanDevice::destroy(const ShaderHandle shader)
    {
        impl_->destroy(shader);
    }

    void VulkanDevice::destroy(const PipelineHandle pipeline)
    {
        impl_->destroy(pipeline);
    }

    std::expected<void, ngen::Error> VulkanDevice::attachWindow(const ngen::platform::NativeWindow &window,
                                                                const ngen::platform::PixelSize size)
    {
        return impl_->attachWindow(window, size);
    }

    void VulkanDevice::onResized(const ngen::platform::PixelSize size)
    {
        impl_->onResized(size);
    }

    void VulkanDevice::onSuspended()
    {
        impl_->onSuspended();
    }

    std::expected<void, ngen::Error> VulkanDevice::onResumed(const ngen::platform::NativeWindow &window,
                                                             const ngen::platform::PixelSize size)
    {
        return impl_->onResumed(window, size);
    }

    std::expected<CommandList *, ngen::Error> VulkanDevice::beginFrame()
    {
        return impl_->beginFrame();
    }

    std::expected<void, ngen::Error> VulkanDevice::endFrame()
    {
        return impl_->endFrame();
    }

    ngen::platform::PixelSize VulkanDevice::swapChainSize() const
    {
        return impl_->swapChainSize();
    }

    void VulkanDevice::waitIdle()
    {
        impl_->waitIdle();
    }
} // namespace ngen::rhi::vulkan
