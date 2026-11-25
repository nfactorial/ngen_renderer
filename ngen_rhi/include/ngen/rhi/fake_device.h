// Spec: rhi. Behaviour changes go through it.
#pragma once

#include <ngen/rhi/command_list.h>
#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/device.h>
#include <ngen/rhi/handle.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace ngen::rhi
{
    // One struct per CommandList call, holding its arguments.
    struct BeginRenderingCommand
    {
        ColorTarget target;

        friend bool operator==(const BeginRenderingCommand &, const BeginRenderingCommand &) = default;
    };

    struct EndRenderingCommand
    {
        friend bool operator==(const EndRenderingCommand &, const EndRenderingCommand &) = default;
    };

    struct SetViewportCommand
    {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;

        friend bool operator==(const SetViewportCommand &, const SetViewportCommand &) = default;
    };

    struct SetScissorCommand
    {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;

        friend bool operator==(const SetScissorCommand &, const SetScissorCommand &) = default;
    };

    struct BindPipelineCommand
    {
        PipelineHandle pipeline;

        friend bool operator==(const BindPipelineCommand &, const BindPipelineCommand &) = default;
    };

    struct BindVertexBufferCommand
    {
        uint32_t binding;
        BufferHandle buffer;
        uint64_t offset;

        friend bool operator==(const BindVertexBufferCommand &, const BindVertexBufferCommand &) = default;
    };

    struct DrawCommand
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

        friend bool operator==(const DrawCommand &, const DrawCommand &) = default;
    };

    using RecordedCommand = std::variant<BeginRenderingCommand, EndRenderingCommand, SetViewportCommand,
                                         SetScissorCommand, BindPipelineCommand, BindVertexBufferCommand, DrawCommand>;

    class FakeCommandList;

    // A Device that keeps resources in handle pools and records commands as values, for testing code built
    // on the RHI. It keeps every requirement of Device and CommandList. Frames "execute" until
    // completeFrames() is called, except that beginFrame completes the oldest when no frame slot is free.
    class FakeDevice final : public Device
    {
    public:
        explicit FakeDevice(DeviceDescription description = {});
        ~FakeDevice() override;

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

        // The commands of each frame ended so far, in order.
        [[nodiscard]] const std::vector<std::vector<RecordedCommand>> &frames() const;

        // Resources not yet freed: live ones, and destroyed ones a frame in flight may still use.
        [[nodiscard]] size_t liveBuffers() const;
        [[nodiscard]] size_t liveShaders() const;
        [[nodiscard]] size_t livePipelines() const;

        // Marks every submitted frame as finished on the "GPU".
        void completeFrames();

    private:
        friend class FakeCommandList;

        // Only the size and usage are kept: writes are range-checked and not stored.
        struct FakeBuffer
        {
            uint64_t size;
            BufferUsage usage;
        };

        struct FakeShader
        {
            std::vector<std::byte> bytecode;
        };

        // A destroyed resource waits until the frame count it was retired at has finished.
        using Retired = std::vector<uint64_t>;

        void report(Severity severity, std::string_view message) const;
        void freeFinishedResources();

        MessageSink sink_;
        HandlePool<BufferTag, FakeBuffer> buffers_;
        HandlePool<ShaderTag, FakeShader> shaders_;
        HandlePool<PipelineTag, GraphicsPipelineDescription> pipelines_;
        Retired retiredBuffers_;
        Retired retiredShaders_;
        Retired retiredPipelines_;

        bool attached_ = false;
        bool suspended_ = false;
        ngen::platform::PixelSize size_{0, 0};
        std::optional<ngen::platform::PixelSize> pendingSize_;

        std::unique_ptr<FakeCommandList> commandList_;
        bool recording_ = false;
        uint64_t framesBegun_ = 0;
        uint64_t framesFinished_ = 0;
        std::vector<std::vector<RecordedCommand>> frames_;
    };
} // namespace ngen::rhi
