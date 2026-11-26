// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/platform/window.h>
#include <ngen/rhi/command_list.h>
#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/device.h>
#include <ngen/rhi/handle.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>

namespace ngen::rhi::vulkan
{
    // Whether the device may present: Expected enables the surface and swap chain extensions so attachWindow
    // may be called; None makes a device that never presents (for tests and offscreen use).
    enum class WindowSupport
    {
        None,
        Expected,
    };

    class DeviceImpl;

    // The Vulkan implementation of Device, owning every Vulkan object it creates. The Vulkan objects live in
    // DeviceImpl, so no Vulkan type appears here.
    class VulkanDevice final : public Device
    {
    public:
        [[nodiscard]] static std::expected<std::unique_ptr<Device>, ngen::Error> create(
            const DeviceDescription &description, WindowSupport windowSupport);

        ~VulkanDevice() override;

        [[nodiscard]] std::expected<BufferHandle, ngen::Error> createBuffer(
            const BufferDescription &description) override;
        [[nodiscard]] std::expected<void, ngen::Error> writeBuffer(BufferHandle buffer, uint64_t offset,
                                                                   std::span<const std::byte> data) override;
        [[nodiscard]] std::expected<ShaderHandle, ngen::Error> createShader(
            const ShaderDescription &description) override;
        [[nodiscard]] std::expected<PipelineHandle, ngen::Error> createPipeline(
            const GraphicsPipelineDescription &description) override;

        void destroy(BufferHandle buffer) override;
        void destroy(ShaderHandle shader) override;
        void destroy(PipelineHandle pipeline) override;

        [[nodiscard]] std::expected<void, ngen::Error> attachWindow(const ngen::platform::NativeWindow &window,
                                                                    ngen::platform::PixelSize size) override;
        void onResized(ngen::platform::PixelSize size) override;
        void onSuspended() override;
        [[nodiscard]] std::expected<void, ngen::Error> onResumed(const ngen::platform::NativeWindow &window,
                                                                 ngen::platform::PixelSize size) override;

        [[nodiscard]] std::expected<CommandList *, ngen::Error> beginFrame() override;
        [[nodiscard]] std::expected<void, ngen::Error> endFrame() override;
        [[nodiscard]] ngen::platform::PixelSize swapChainSize() const override;
        void waitIdle() override;

    private:
        explicit VulkanDevice(std::unique_ptr<DeviceImpl> impl);

        std::unique_ptr<DeviceImpl> impl_;
    };
} // namespace ngen::rhi::vulkan
