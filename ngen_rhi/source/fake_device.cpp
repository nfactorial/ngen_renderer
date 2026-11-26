// Spec: rhi. Behaviour changes go through it.
#include <ngen/rhi/fake_device.h>

#include <string>
#include <utility>
#include <variant>

namespace ngen::rhi
{
    // Records its calls as RecordedCommand values, after checking call order and handles.
    class FakeCommandList final : public CommandList
    {
    public:
        explicit FakeCommandList(FakeDevice &device) : device_(device)
        {
        }

        void begin()
        {
            commands_.clear();
            rendering_ = false;
            recording_ = true;
            swapChainImageState_ = ResourceState::Undefined;
        }

        // Hands over the recorded commands; reports a rendering pass left open. The image's move to Present is
        // not recorded.
        std::vector<RecordedCommand> finish()
        {
            if (rendering_)
            {
                device_.report(Severity::Error, "endFrame: beginRendering has no matching endRendering");
                rendering_ = false;
            }
            recording_ = false;
            return std::move(commands_);
        }

        void barrier(const Barrier &barrier) override
        {
            if (!requireRecording("barrier"))
            {
                return;
            }
            if (rendering_)
            {
                device_.report(Severity::Error, "barrier: called inside beginRendering and endRendering");
                return;
            }
            ResourceState &state = trackedState(barrier.target);
            if (auto valid = validate(barrier, state); !valid)
            {
                device_.report(Severity::Error, valid.error().message);
                return;
            }
            state = barrier.after;
            commands_.push_back(barrier);
        }

        void beginRendering(const ColorTarget &target) override
        {
            if (!requireRecording("beginRendering"))
            {
                return;
            }
            if (rendering_)
            {
                device_.report(Severity::Error, "beginRendering: already rendering");
                return;
            }
            // The image's own move to ColorAttachment is recorded as the barrier it stands for.
            if (swapChainImageState_ != ResourceState::ColorAttachment)
            {
                commands_.push_back(
                    Barrier{SwapChainImage{}, swapChainImageState_, ResourceState::ColorAttachment});
                swapChainImageState_ = ResourceState::ColorAttachment;
            }
            rendering_ = true;
            commands_.push_back(BeginRenderingCommand{target});
        }

        void endRendering() override
        {
            if (!requireRecording("endRendering"))
            {
                return;
            }
            if (!rendering_)
            {
                device_.report(Severity::Error, "endRendering: not rendering");
                return;
            }
            rendering_ = false;
            commands_.push_back(EndRenderingCommand{});
        }

        void setViewport(const float x, const float y, const float width, const float height, const float minDepth,
                         const float maxDepth) override
        {
            if (!requireRecording("setViewport"))
            {
                return;
            }
            commands_.push_back(SetViewportCommand{x, y, width, height, minDepth, maxDepth});
        }

        void setScissor(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) override
        {
            if (!requireRecording("setScissor"))
            {
                return;
            }
            commands_.push_back(SetScissorCommand{x, y, width, height});
        }

        void bindPipeline(const PipelineHandle pipeline) override
        {
            if (!requireRecording("bindPipeline") || !requireRendering("bindPipeline"))
            {
                return;
            }
            if (device_.pipelines_.get(pipeline) == nullptr)
            {
                device_.report(Severity::Error, "bindPipeline: the pipeline handle is null or stale");
                return;
            }
            commands_.push_back(BindPipelineCommand{pipeline});
        }

        void bindVertexBuffer(const uint32_t binding, const BufferHandle buffer, const uint64_t offset) override
        {
            if (!requireRecording("bindVertexBuffer") || !requireRendering("bindVertexBuffer"))
            {
                return;
            }
            const FakeDevice::FakeBuffer *target = device_.buffers_.get(buffer);
            if (target == nullptr)
            {
                device_.report(Severity::Error, "bindVertexBuffer: the buffer handle is null or stale");
                return;
            }
            if ((target->usage & BufferUsage::Vertex) == static_cast<BufferUsage>(0))
            {
                device_.report(Severity::Error,
                               "bindVertexBuffer: the buffer was not created with BufferUsage::Vertex");
                return;
            }
            if (offset >= target->size)
            {
                device_.report(Severity::Error, "bindVertexBuffer: the offset is not inside the buffer");
                return;
            }
            commands_.push_back(BindVertexBufferCommand{binding, buffer, offset});
        }

        void draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
                  const uint32_t firstInstance) override
        {
            if (!requireRecording("draw") || !requireRendering("draw"))
            {
                return;
            }
            commands_.push_back(DrawCommand{vertexCount, instanceCount, firstVertex, firstInstance});
        }

    private:
        ResourceState &trackedState(const BarrierTarget &target)
        {
            return std::visit([this](const SwapChainImage &) -> ResourceState & { return swapChainImageState_; },
                              target);
        }

        bool requireRecording(const char *const command)
        {
            if (recording_)
            {
                return true;
            }
            device_.report(Severity::Error, std::string(command) + " called after endFrame");
            return false;
        }

        bool requireRendering(const char *const command)
        {
            if (rendering_)
            {
                return true;
            }
            device_.report(Severity::Error, std::string(command) + ": called outside beginRendering and endRendering");
            return false;
        }

        FakeDevice &device_;
        std::vector<RecordedCommand> commands_;
        bool rendering_ = false;
        bool recording_ = false;
        ResourceState swapChainImageState_ = ResourceState::Undefined;
    };

    namespace
    {
        std::unexpected<ngen::Error> invalid(std::string message)
        {
            return std::unexpected(ngen::makeError(ngen::ErrorCode::InvalidArgument, std::move(message)));
        }

        template <typename Tag, typename T>
        void retire(HandlePool<Tag, T> &pool, const Handle<Tag> handle, const uint64_t framesBegun,
                    std::vector<uint64_t> &retired)
        {
            if (pool.remove(handle).has_value())
            {
                retired.push_back(framesBegun);
            }
        }
    } // namespace

    FakeDevice::FakeDevice(DeviceDescription description)
        : sink_(std::move(description.messageSink)), commandList_(std::make_unique<FakeCommandList>(*this))
    {
        if (!sink_)
        {
            sink_ = makeDefaultMessageSink();
        }
    }

    FakeDevice::~FakeDevice() = default;

    void FakeDevice::report(const Severity severity, const std::string_view message) const
    {
        sink_(severity, message);
    }

    std::expected<BufferHandle, ngen::Error> FakeDevice::createBuffer(const BufferDescription &description)
    {
        if (auto valid = validate(description); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }
        return buffers_.insert(FakeBuffer{description.size, description.usage});
    }

    std::expected<void, ngen::Error> FakeDevice::writeBuffer(const BufferHandle buffer, const uint64_t offset,
                                                             const std::span<const std::byte> data)
    {
        FakeBuffer *target = buffers_.get(buffer);
        if (target == nullptr)
        {
            return invalid("writeBuffer: the buffer handle is null or stale");
        }
        if (offset > target->size || data.size() > target->size - offset)
        {
            return invalid("writeBuffer: the range offset + size is past the end of the buffer");
        }
        return {};
    }

    std::expected<ShaderHandle, ngen::Error> FakeDevice::createShader(const ShaderDescription &description)
    {
        if (auto valid = validate(description); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }
        return shaders_.insert(FakeShader{std::vector<std::byte>(description.bytecode.begin(), description.bytecode.end())});
    }

    std::expected<PipelineHandle, ngen::Error> FakeDevice::createPipeline(const GraphicsPipelineDescription &description)
    {
        if (auto valid = validate(description); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }
        if (shaders_.get(description.vertex.shader) == nullptr)
        {
            return invalid("GraphicsPipelineDescription.vertex.shader is a null or stale handle");
        }
        if (shaders_.get(description.fragment.shader) == nullptr)
        {
            return invalid("GraphicsPipelineDescription.fragment.shader is a null or stale handle");
        }
        return pipelines_.insert(description);
    }

    void FakeDevice::destroy(const BufferHandle buffer)
    {
        retire(buffers_, buffer, framesBegun_, retiredBuffers_);
    }

    void FakeDevice::destroy(const ShaderHandle shader)
    {
        retire(shaders_, shader, framesBegun_, retiredShaders_);
    }

    void FakeDevice::destroy(const PipelineHandle pipeline)
    {
        retire(pipelines_, pipeline, framesBegun_, retiredPipelines_);
    }

    std::expected<void, ngen::Error> FakeDevice::attachWindow(const ngen::platform::NativeWindow &,
                                                              const ngen::platform::PixelSize size)
    {
        if (attached_)
        {
            return invalid("attachWindow: the device already has a swap chain");
        }
        attached_ = true;
        suspended_ = false;
        size_ = size;
        pendingSize_.reset();
        return {};
    }

    void FakeDevice::onResized(const ngen::platform::PixelSize size)
    {
        if (attached_ && !suspended_)
        {
            pendingSize_ = size;
        }
    }

    void FakeDevice::onSuspended()
    {
        if (attached_)
        {
            suspended_ = true;
        }
    }

    std::expected<void, ngen::Error> FakeDevice::onResumed(const ngen::platform::NativeWindow &,
                                                           const ngen::platform::PixelSize size)
    {
        if (!attached_)
        {
            return invalid("onResumed: no window is attached");
        }
        suspended_ = false;
        size_ = size;
        pendingSize_.reset();
        return {};
    }

    std::expected<CommandList *, ngen::Error> FakeDevice::beginFrame()
    {
        if (recording_)
        {
            report(Severity::Error, "beginFrame: a frame is already being recorded");
            return invalid("beginFrame: a frame is already being recorded");
        }

        freeFinishedResources();

        if (attached_ && !suspended_ && pendingSize_.has_value())
        {
            size_ = *pendingSize_;
            pendingSize_.reset();
        }
        if (!attached_ || suspended_ || size_.width == 0 || size_.height == 0)
        {
            return nullptr;
        }

        // No frame slot is free: the oldest frame finishes.
        if (framesBegun_ - framesFinished_ >= kMaxFramesInFlight)
        {
            ++framesFinished_;
            freeFinishedResources();
        }

        ++framesBegun_;
        recording_ = true;
        commandList_->begin();
        return commandList_.get();
    }

    std::expected<void, ngen::Error> FakeDevice::endFrame()
    {
        if (!recording_)
        {
            report(Severity::Error, "endFrame: no frame is being recorded");
            return invalid("endFrame: no frame is being recorded");
        }
        frames_.push_back(commandList_->finish());
        recording_ = false;
        return {};
    }

    ngen::platform::PixelSize FakeDevice::swapChainSize() const
    {
        // No swap chain while suspended or while either dimension is 0.
        if (!attached_ || suspended_ || size_.width == 0 || size_.height == 0)
        {
            return ngen::platform::PixelSize{0, 0};
        }
        return size_;
    }

    void FakeDevice::waitIdle()
    {
        completeFrames();
    }

    const std::vector<std::vector<RecordedCommand>> &FakeDevice::frames() const
    {
        return frames_;
    }

    size_t FakeDevice::liveBuffers() const
    {
        return buffers_.size() + retiredBuffers_.size();
    }

    size_t FakeDevice::liveShaders() const
    {
        return shaders_.size() + retiredShaders_.size();
    }

    size_t FakeDevice::livePipelines() const
    {
        return pipelines_.size() + retiredPipelines_.size();
    }

    void FakeDevice::completeFrames()
    {
        framesFinished_ = frames_.size();
    }

    void FakeDevice::freeFinishedResources()
    {
        const auto isFinished = [this](const uint64_t retiredAt) { return retiredAt <= framesFinished_; };
        std::erase_if(retiredBuffers_, isFinished);
        std::erase_if(retiredShaders_, isFinished);
        std::erase_if(retiredPipelines_, isFinished);
    }
} // namespace ngen::rhi
