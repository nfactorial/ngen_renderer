// Spec: rhi. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/platform/window.h>
#include <ngen/rhi/command_list.h>
#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/handle.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace ngen::rhi
{
    // Up to this many frames are recorded or executing at once.
    constexpr uint32_t kMaxFramesInFlight = 2;

    enum class Severity
    {
        Info,
        Warning,
        Error,
    };

    using MessageSink = std::function<void(Severity, std::string_view)>;

    // Writes warnings and errors to stderr and drops info. Used when a description has no sink.
    MessageSink makeDefaultMessageSink();

    struct DeviceDescription
    {
        std::string applicationName;
        bool enableValidation = false;
        MessageSink messageSink; // empty means makeDefaultMessageSink()
    };

    // The interface a graphics backend implements. All calls are made on the thread that created the device.
    class Device
    {
    public:
        Device() = default;
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        // An implementation waits for the GPU, then frees every resource still alive.
        virtual ~Device() = default;

        [[nodiscard]] virtual std::expected<BufferHandle, ngen::Error> createBuffer(
            const BufferDescription &description) = 0;

        // Copies into the buffer at once; the caller must not write a range a frame in flight is reading.
        [[nodiscard]] virtual std::expected<void, ngen::Error> writeBuffer(BufferHandle buffer, uint64_t offset,
                                                                           std::span<const std::byte> data) = 0;

        [[nodiscard]] virtual std::expected<ShaderHandle, ngen::Error> createShader(
            const ShaderDescription &description) = 0;

        [[nodiscard]] virtual std::expected<PipelineHandle, ngen::Error> createPipeline(
            const GraphicsPipelineDescription &description) = 0;

        // Retire the handle at once and free the resource after every frame begun before the call has
        // finished. A null or stale handle is ignored.
        virtual void destroy(BufferHandle buffer) = 0;
        virtual void destroy(ShaderHandle shader) = 0;
        virtual void destroy(PipelineHandle pipeline) = 0;

        // Creates the swap chain; a device holds at most one.
        [[nodiscard]] virtual std::expected<void, ngen::Error> attachWindow(const ngen::platform::NativeWindow &window,
                                                                            ngen::platform::PixelSize size) = 0;

        // The application forwards the window's events.
        virtual void onResized(ngen::platform::PixelSize size) = 0;
        virtual void onSuspended() = 0;
        [[nodiscard]] virtual std::expected<void, ngen::Error> onResumed(const ngen::platform::NativeWindow &window,
                                                                         ngen::platform::PixelSize size) = 0;

        // Waits until a frame slot is free and returns its command list, recording. nullptr (no error) when
        // there is nothing to present to: no window attached, suspended, or a 0-sized window. The command
        // list is valid until endFrame.
        [[nodiscard]] virtual std::expected<CommandList *, ngen::Error> beginFrame() = 0;

        // Ends recording, submits and presents. Called only after a beginFrame that returned a command list.
        [[nodiscard]] virtual std::expected<void, ngen::Error> endFrame() = 0;

        // The size of the images being presented; {0, 0} with no swap chain.
        [[nodiscard]] virtual ngen::platform::PixelSize swapChainSize() const = 0;

        // Blocks until the GPU has finished all submitted work.
        virtual void waitIdle() = 0;
    };
} // namespace ngen::rhi
