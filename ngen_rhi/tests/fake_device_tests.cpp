// Spec: rhi. Behaviour changes go through it.
#include <ngen/rhi/fake_device.h>

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

using namespace ngen::rhi;
using ngen::platform::NativeWindow;
using ngen::platform::PixelSize;
using ngen::platform::Win32Handles;

namespace
{
    struct Message
    {
        Severity severity;
        std::string text;
    };

    const std::array<std::byte, 4> kBytecode{};

    class FakeDeviceTest : public ::testing::Test
    {
    protected:
        FakeDeviceTest()
            : device([this] {
                  DeviceDescription description;
                  description.messageSink = [this](const Severity severity, const std::string_view text) {
                      messages.push_back({severity, std::string(text)});
                  };
                  return FakeDevice(std::move(description));
              }())
        {
        }

        size_t errorCount() const
        {
            size_t count = 0;
            for (const Message &message : messages)
            {
                count += message.severity == Severity::Error ? 1 : 0;
            }
            return count;
        }

        void attach(const PixelSize size = {640, 480})
        {
            ASSERT_TRUE(device.attachWindow(NativeWindow{Win32Handles{}}, size).has_value());
        }

        // Begins a frame; nullptr (after a reported failure) when no command list came back.
        CommandList *begin()
        {
            auto frame = device.beginFrame();
            EXPECT_TRUE(frame.has_value());
            return frame.value_or(nullptr);
        }

        ShaderHandle makeShader()
        {
            auto shader = device.createShader(ShaderDescription{kBytecode});
            EXPECT_TRUE(shader.has_value());
            return shader.value_or(ShaderHandle{});
        }

        PipelineHandle makePipeline()
        {
            GraphicsPipelineDescription description;
            description.vertex = {makeShader(), "main"};
            description.fragment = {makeShader(), "main"};
            description.bindings = {{0, 12}};
            description.attributes = {{0, 0, Format::R32G32B32Float, 0}};
            auto pipeline = device.createPipeline(description);
            EXPECT_TRUE(pipeline.has_value());
            return pipeline.value_or(PipelineHandle{});
        }

        BufferHandle makeBuffer(const uint64_t size = 64, const BufferUsage usage = BufferUsage::Vertex)
        {
            auto buffer = device.createBuffer(BufferDescription{size, usage});
            EXPECT_TRUE(buffer.has_value());
            return buffer.value_or(BufferHandle{});
        }

        std::vector<Message> messages;
        FakeDevice device;
    };
} // namespace

// Resource creation and its validation

TEST_F(FakeDeviceTest, CreateBufferWithInvalidDescriptionIsInvalidArgumentNamingField)
{
    const auto result = device.createBuffer(BufferDescription{0, BufferUsage::Vertex});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_NE(result.error().message.find("size"), std::string::npos);
    EXPECT_EQ(device.liveBuffers(), 0u);
}

TEST_F(FakeDeviceTest, CreateBufferWithEmptyUsageIsInvalidArgumentNamingField)
{
    const auto result = device.createBuffer(BufferDescription{16, static_cast<BufferUsage>(0)});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_NE(result.error().message.find("usage"), std::string::npos);
}

TEST_F(FakeDeviceTest, CreatePipelineWithDuplicateLocationIsInvalidArgumentNamingField)
{
    GraphicsPipelineDescription description;
    description.vertex = {makeShader(), "main"};
    description.fragment = {makeShader(), "main"};
    description.bindings = {{0, 24}};
    description.attributes = {{0, 0, Format::R32G32Float, 0}, {0, 0, Format::R32G32Float, 8}};

    const auto result = device.createPipeline(description);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_NE(result.error().message.find("attributes[1].location"), std::string::npos);
}

TEST_F(FakeDeviceTest, CreateShaderWithEmptyBytecodeIsInvalidArgument)
{
    const auto result = device.createShader(ShaderDescription{});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_NE(result.error().message.find("bytecode"), std::string::npos);
}

TEST_F(FakeDeviceTest, CreatePipelineNamingMissingBindingIsInvalidArgument)
{
    GraphicsPipelineDescription description;
    description.vertex = {makeShader(), "main"};
    description.fragment = {makeShader(), "main"};
    description.bindings = {{0, 12}};
    description.attributes = {{0, 1, Format::R32G32B32Float, 0}};

    const auto result = device.createPipeline(description);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_NE(result.error().message.find("attributes[0].binding"), std::string::npos);
    EXPECT_EQ(device.livePipelines(), 0u);
}

TEST_F(FakeDeviceTest, CreatePipelineWithNullOrStaleShaderIsInvalidArgument)
{
    const ShaderHandle live = makeShader();
    const ShaderHandle stale = makeShader();
    device.destroy(stale);

    GraphicsPipelineDescription description;
    description.vertex = {live, "main"};
    description.fragment = {ShaderHandle{}, "main"};
    auto result = device.createPipeline(description);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);

    description.fragment = {stale, "main"};
    result = device.createPipeline(description);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);

    description.fragment = {live, "main"};
    EXPECT_TRUE(device.createPipeline(description).has_value());
}

TEST_F(FakeDeviceTest, WriteBufferInRangeSucceeds)
{
    const BufferHandle buffer = makeBuffer(8);
    const std::array<std::byte, 4> data{};
    EXPECT_TRUE(device.writeBuffer(buffer, 4, data).has_value());
}

TEST_F(FakeDeviceTest, WriteBufferPastTheEndIsInvalidArgument)
{
    const BufferHandle buffer = makeBuffer(8);
    const std::array<std::byte, 4> data{};
    const auto result = device.writeBuffer(buffer, 5, data);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);

    EXPECT_FALSE(device.writeBuffer(buffer, UINT64_MAX, data).has_value());
}

TEST_F(FakeDeviceTest, WriteBufferToStaleHandleIsInvalidArgument)
{
    const BufferHandle buffer = makeBuffer();
    device.destroy(buffer);
    const std::array<std::byte, 4> data{};
    const auto result = device.writeBuffer(buffer, 0, data);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
}

// Frames and the swap chain

TEST_F(FakeDeviceTest, BeginFrameReturnsNullptrBeforeAttachWindow)
{
    const auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
    EXPECT_EQ(device.swapChainSize(), (PixelSize{0, 0}));
    EXPECT_EQ(errorCount(), 0u);
}

TEST_F(FakeDeviceTest, AttachWindowTwiceIsInvalidArgument)
{
    attach();
    const auto result = device.attachWindow(NativeWindow{Win32Handles{}}, PixelSize{10, 10});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
}

TEST_F(FakeDeviceTest, BeginFrameWithZeroSizedWindowReturnsNullptr)
{
    attach({0, 0});
    const auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
}

TEST_F(FakeDeviceTest, ResizeAppliesAtTheNextBeginFrame)
{
    attach({640, 480});
    EXPECT_EQ(device.swapChainSize(), (PixelSize{640, 480}));

    device.onResized({800, 600});
    (void)begin();
    EXPECT_EQ(device.swapChainSize(), (PixelSize{800, 600}));
    ASSERT_TRUE(device.endFrame().has_value());
}

TEST_F(FakeDeviceTest, WindowWithOneZeroDimensionHasNoSwapChain)
{
    attach({0, 480});
    EXPECT_EQ(device.swapChainSize(), (PixelSize{0, 0}));
    const auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
}

TEST_F(FakeDeviceTest, ResizeToZeroMakesBeginFrameReturnNullptr)
{
    attach();
    device.onResized({0, 0});
    const auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
}

TEST_F(FakeDeviceTest, SuspendThenResumeGatesBeginFrame)
{
    attach();
    device.onSuspended();
    auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
    EXPECT_EQ(device.swapChainSize(), (PixelSize{0, 0}));

    ASSERT_TRUE(device.onResumed(NativeWindow{Win32Handles{}}, PixelSize{320, 200}).has_value());
    (void)begin();
    EXPECT_EQ(device.swapChainSize(), (PixelSize{320, 200}));
}

TEST_F(FakeDeviceTest, ResumeWithoutAttachedWindowIsInvalidArgument)
{
    const auto result = device.onResumed(NativeWindow{Win32Handles{}}, PixelSize{320, 200});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
}

TEST_F(FakeDeviceTest, BeginFrameWhileRecordingReportsErrorAndReturnsInvalidArgument)
{
    attach();
    (void)begin();
    const auto result = device.beginFrame();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
}

TEST_F(FakeDeviceTest, EndFrameWithoutFrameReportsErrorAndReturnsInvalidArgument)
{
    attach();
    const auto result = device.endFrame();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
}

// Recording

TEST_F(FakeDeviceTest, RecordedFrameHoldsExactlyTheCallsWithTheirArguments)
{
    attach();
    const PipelineHandle pipeline = makePipeline();
    const BufferHandle buffer = makeBuffer();

    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->beginRendering(ColorTarget{});
    commands->bindPipeline(pipeline);
    commands->bindVertexBuffer(0, buffer);
    commands->draw(3);
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    ASSERT_EQ(device.frames().size(), 1u);
    const std::vector<RecordedCommand> expected{
        Barrier{SwapChainImage{}, ResourceState::Undefined, ResourceState::ColorAttachment},
        BeginRenderingCommand{ColorTarget{}},
        BindPipelineCommand{pipeline},
        BindVertexBufferCommand{0, buffer, 0},
        DrawCommand{3, 1, 0, 0},
        EndRenderingCommand{},
    };
    EXPECT_EQ(device.frames()[0], expected);
    EXPECT_TRUE(messages.empty());
}

TEST_F(FakeDeviceTest, ViewportAndScissorAreRecordedWithTheirArguments)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->beginRendering(ColorTarget{LoadOp::Clear, StoreOp::Store, {1, 0, 0, 1}});
    commands->setViewport(1, 2, 3, 4);
    commands->setScissor(-5, 6, 7, 8);
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    const std::vector<RecordedCommand> expected{
        Barrier{SwapChainImage{}, ResourceState::Undefined, ResourceState::ColorAttachment},
        BeginRenderingCommand{ColorTarget{LoadOp::Clear, StoreOp::Store, {1, 0, 0, 1}}},
        SetViewportCommand{1, 2, 3, 4, 0, 1},
        SetScissorCommand{-5, 6, 7, 8},
        EndRenderingCommand{},
    };
    EXPECT_EQ(device.frames()[0], expected);
}

TEST_F(FakeDeviceTest, FramesAreKeptInOrder)
{
    attach();
    for (const uint32_t count : {3u, 6u})
    {
        CommandList *const commands = begin();
        ASSERT_NE(commands, nullptr);
        commands->beginRendering(ColorTarget{});
        commands->draw(count);
        commands->endRendering();
        ASSERT_TRUE(device.endFrame().has_value());
        device.completeFrames();
    }

    ASSERT_EQ(device.frames().size(), 2u);
    EXPECT_EQ(std::get<DrawCommand>(device.frames()[0][2]).vertexCount, 3u);
    EXPECT_EQ(std::get<DrawCommand>(device.frames()[1][2]).vertexCount, 6u);
}

TEST_F(FakeDeviceTest, DrawBeforeBeginRenderingRecordsNothingAndSendsOneError)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->draw(3);
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_TRUE(device.frames()[0].empty());
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
}

TEST_F(FakeDeviceTest, BindCommandsOutsideRenderingAreSkippedWithAnError)
{
    attach();
    const PipelineHandle pipeline = makePipeline();
    const BufferHandle buffer = makeBuffer();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->bindPipeline(pipeline);
    commands->bindVertexBuffer(0, buffer);
    commands->beginRendering(ColorTarget{});
    commands->endRendering();
    commands->draw(3);
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_EQ(device.frames()[0].size(), 3u); // only beginRendering (with its barrier) and endRendering
    EXPECT_EQ(errorCount(), 3u);
}

TEST_F(FakeDeviceTest, NullOrStaleHandlesSkipTheCommandWithAnErrorNamingIt)
{
    attach();
    const PipelineHandle stalePipeline = makePipeline();
    const BufferHandle staleBuffer = makeBuffer();
    device.destroy(stalePipeline);
    device.destroy(staleBuffer);

    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->beginRendering(ColorTarget{});
    commands->bindPipeline(PipelineHandle{});
    commands->bindPipeline(stalePipeline);
    commands->bindVertexBuffer(0, BufferHandle{});
    commands->bindVertexBuffer(0, staleBuffer);
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_EQ(device.frames()[0].size(), 3u); // beginRendering (with its barrier) and endRendering
    ASSERT_EQ(messages.size(), 4u);
    EXPECT_NE(messages[0].text.find("pipeline"), std::string::npos);
    EXPECT_NE(messages[1].text.find("pipeline"), std::string::npos);
    EXPECT_NE(messages[2].text.find("buffer"), std::string::npos);
    EXPECT_NE(messages[3].text.find("buffer"), std::string::npos);
    for (const Message &message : messages)
    {
        EXPECT_EQ(message.severity, Severity::Error);
    }
}

TEST_F(FakeDeviceTest, BindVertexBufferWithoutVertexUsageIsSkippedWithAnError)
{
    attach();
    const BufferHandle uniformOnly = makeBuffer(64, BufferUsage::Uniform);
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->beginRendering(ColorTarget{});
    commands->bindVertexBuffer(0, uniformOnly);
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_EQ(device.frames()[0].size(), 3u); // beginRendering (with its barrier) and endRendering
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
    EXPECT_NE(messages[0].text.find("Vertex"), std::string::npos);
}

TEST_F(FakeDeviceTest, BindVertexBufferWithOffsetAtOrPastTheEndIsSkippedWithAnError)
{
    attach();
    const BufferHandle buffer = makeBuffer(64);
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->beginRendering(ColorTarget{});
    commands->bindVertexBuffer(0, buffer, 64);
    commands->bindVertexBuffer(0, buffer, 1000);
    commands->bindVertexBuffer(0, buffer, 63);
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_EQ(device.frames()[0].size(), 4u); // begin (with its barrier), the offset 63 bind, end
    EXPECT_EQ(errorCount(), 2u);
}

TEST_F(FakeDeviceTest, CommandsAfterEndFrameAreSkippedWithAnError)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    ASSERT_TRUE(device.endFrame().has_value());

    commands->beginRendering(ColorTarget{});
    commands->setViewport(0, 0, 1, 1);
    commands->setScissor(0, 0, 1, 1);
    commands->draw(3);
    commands->endRendering();

    EXPECT_EQ(errorCount(), 5u);
    EXPECT_NE(messages[0].text.find("after endFrame"), std::string::npos);
    EXPECT_TRUE(device.frames()[0].empty());

    // The next frame records again.
    CommandList *const next = begin();
    ASSERT_NE(next, nullptr);
    next->beginRendering(ColorTarget{});
    next->draw(1);
    next->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());
    EXPECT_EQ(errorCount(), 5u);
    EXPECT_EQ(device.frames()[1].size(), 4u); // begin (with its barrier), draw, end
}

// Barriers

namespace
{
    Barrier swapChainBarrier(const ResourceState before, const ResourceState after)
    {
        return Barrier{SwapChainImage{}, before, after};
    }
} // namespace

TEST_F(FakeDeviceTest, TwoPassesWithExplicitBarriersRecordBothBarriersAndNoOther)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment));
    commands->beginRendering(ColorTarget{});
    commands->endRendering();
    commands->barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::ColorAttachment));
    commands->beginRendering(ColorTarget{LoadOp::Load, StoreOp::Store});
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    const std::vector<RecordedCommand> expected{
        swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment),
        BeginRenderingCommand{ColorTarget{}},
        EndRenderingCommand{},
        swapChainBarrier(ResourceState::ColorAttachment, ResourceState::ColorAttachment),
        BeginRenderingCommand{ColorTarget{LoadOp::Load, StoreOp::Store}},
        EndRenderingCommand{},
    };
    EXPECT_EQ(device.frames()[0], expected);
    EXPECT_TRUE(messages.empty());
}

TEST_F(FakeDeviceTest, BarrierFromTheWrongStateRecordsNothingAndNamesBothStates)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::Present));
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_TRUE(device.frames()[0].empty());
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
    EXPECT_NE(messages[0].text.find("Undefined"), std::string::npos) << messages[0].text;
    EXPECT_NE(messages[0].text.find("ColorAttachment"), std::string::npos) << messages[0].text;
}

TEST_F(FakeDeviceTest, BarrierInsideRenderingRecordsNothingAndSendsOneError)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->beginRendering(ColorTarget{});
    commands->barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::ColorAttachment));
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    const std::vector<RecordedCommand> expected{
        swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment),
        BeginRenderingCommand{ColorTarget{}},
        EndRenderingCommand{},
    };
    EXPECT_EQ(device.frames()[0], expected);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
}

TEST_F(FakeDeviceTest, BarrierToUndefinedIsSkippedWithAnError)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::Undefined));
    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment));
    commands->barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::Undefined));
    ASSERT_TRUE(device.endFrame().has_value());

    const std::vector<RecordedCommand> expected{
        swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment),
    };
    EXPECT_EQ(device.frames()[0], expected);
    ASSERT_EQ(messages.size(), 2u);
    for (const Message &message : messages)
    {
        EXPECT_EQ(message.severity, Severity::Error);
        EXPECT_NE(message.text.find("Undefined"), std::string::npos) << message.text;
    }
}

TEST_F(FakeDeviceTest, BarrierMovingTheSwapChainImageToOrFromShaderReadIsSkippedWithAnError)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ShaderRead));
    commands->barrier(swapChainBarrier(ResourceState::ShaderRead, ResourceState::ColorAttachment));
    ASSERT_TRUE(device.endFrame().has_value());

    EXPECT_TRUE(device.frames()[0].empty());
    ASSERT_EQ(messages.size(), 2u);
    for (const Message &message : messages)
    {
        EXPECT_EQ(message.severity, Severity::Error);
        EXPECT_NE(message.text.find("ShaderRead"), std::string::npos) << message.text;
    }
}

TEST_F(FakeDeviceTest, EachBarrierLeavesTheImageInItsAfterState)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::Present));
    commands->barrier(swapChainBarrier(ResourceState::Present, ResourceState::ColorAttachment));
    commands->barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::Present));
    ASSERT_TRUE(device.endFrame().has_value());

    // endFrame's transition to Present is not recorded.
    const std::vector<RecordedCommand> expected{
        swapChainBarrier(ResourceState::Undefined, ResourceState::Present),
        swapChainBarrier(ResourceState::Present, ResourceState::ColorAttachment),
        swapChainBarrier(ResourceState::ColorAttachment, ResourceState::Present),
    };
    EXPECT_EQ(device.frames()[0], expected);
    EXPECT_TRUE(messages.empty());
}

TEST_F(FakeDeviceTest, BeginRenderingMovesTheImageFromItsTrackedStateOnlyWhenNotAColorAttachment)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::Present));
    commands->beginRendering(ColorTarget{});
    commands->endRendering();
    commands->beginRendering(ColorTarget{}); // already a ColorAttachment: no barrier
    commands->endRendering();
    ASSERT_TRUE(device.endFrame().has_value());

    const std::vector<RecordedCommand> expected{
        swapChainBarrier(ResourceState::Undefined, ResourceState::Present),
        swapChainBarrier(ResourceState::Present, ResourceState::ColorAttachment),
        BeginRenderingCommand{ColorTarget{}},
        EndRenderingCommand{},
        BeginRenderingCommand{ColorTarget{}},
        EndRenderingCommand{},
    };
    EXPECT_EQ(device.frames()[0], expected);
    EXPECT_TRUE(messages.empty());
}

TEST_F(FakeDeviceTest, TheImageIsUndefinedAtEveryBeginFrame)
{
    attach();
    for (int frame = 0; frame < 3; ++frame)
    {
        CommandList *const commands = begin();
        ASSERT_NE(commands, nullptr);
        // The frame before left the image a ColorAttachment (and endFrame made it Present).
        commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment));
        commands->beginRendering(ColorTarget{});
        commands->endRendering();
        ASSERT_TRUE(device.endFrame().has_value());
        device.completeFrames();
    }

    const std::vector<RecordedCommand> expected{
        swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment),
        BeginRenderingCommand{ColorTarget{}},
        EndRenderingCommand{},
    };
    ASSERT_EQ(device.frames().size(), 3u);
    for (const std::vector<RecordedCommand> &frame : device.frames())
    {
        EXPECT_EQ(frame, expected);
    }
    EXPECT_TRUE(messages.empty());
}

TEST_F(FakeDeviceTest, BarrierAfterEndFrameIsSkippedWithAnError)
{
    attach();
    CommandList *const commands = begin();
    ASSERT_NE(commands, nullptr);
    ASSERT_TRUE(device.endFrame().has_value());

    commands->barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment));
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].severity, Severity::Error);
    EXPECT_NE(messages[0].text.find("after endFrame"), std::string::npos);
    EXPECT_TRUE(device.frames()[0].empty());
}

// Deferred destruction

TEST_F(FakeDeviceTest, BufferDestroyedDuringFrameIsFreedByNextBeginFrameAfterCompletion)
{
    attach();
    const BufferHandle buffer = makeBuffer();
    (void)begin();
    device.destroy(buffer);
    ASSERT_TRUE(device.endFrame().has_value());
    EXPECT_EQ(device.liveBuffers(), 1u);

    device.completeFrames();
    EXPECT_EQ(device.liveBuffers(), 1u); // freed by beginFrame, not by completion
    (void)begin();
    EXPECT_EQ(device.liveBuffers(), 0u);
}

TEST_F(FakeDeviceTest, DestroyedResourceOutlivesFramesStillExecutingAcrossBeginFrames)
{
    attach();
    const ShaderHandle shader = makeShader();
    (void)begin();
    device.destroy(shader);
    ASSERT_TRUE(device.endFrame().has_value());

    (void)begin(); // frame 2: frame 1 is still executing
    ASSERT_TRUE(device.endFrame().has_value());
    EXPECT_EQ(device.liveShaders(), 1u);

    (void)begin(); // frame 3: both slots are busy, so frame 1 finishes
    EXPECT_EQ(device.liveShaders(), 0u);
}

TEST_F(FakeDeviceTest, ResourceDestroyedAfterAFrameBeganOutlivesThatFrame)
{
    attach();
    const PipelineHandle pipeline = makePipeline();
    (void)begin();
    ASSERT_TRUE(device.endFrame().has_value());
    device.completeFrames();

    (void)begin(); // frame 2 begun, then the pipeline is destroyed
    device.destroy(pipeline);
    ASSERT_TRUE(device.endFrame().has_value());
    EXPECT_EQ(device.livePipelines(), 1u);
    (void)begin(); // frame 2 still executing
    EXPECT_EQ(device.livePipelines(), 1u);
    ASSERT_TRUE(device.endFrame().has_value());
    device.completeFrames();
    (void)begin();
    EXPECT_EQ(device.livePipelines(), 0u);
}

TEST_F(FakeDeviceTest, ResourceDestroyedWithNoFrameInFlightIsFreedByNextBeginFrame)
{
    attach();
    const BufferHandle buffer = makeBuffer();
    (void)begin();
    ASSERT_TRUE(device.endFrame().has_value());
    device.completeFrames();

    device.destroy(buffer);
    EXPECT_EQ(device.liveBuffers(), 1u);
    (void)begin();
    EXPECT_EQ(device.liveBuffers(), 0u);
}

TEST_F(FakeDeviceTest, ResourceDestroyedBeforeAnyFrameIsFreedByNextBeginFrame)
{
    attach();
    device.destroy(makeBuffer());
    (void)begin();
    EXPECT_EQ(device.liveBuffers(), 0u);
}

TEST_F(FakeDeviceTest, DestroyRetiresTheHandleAtOnceAndIgnoresNullAndStaleHandles)
{
    const BufferHandle buffer = makeBuffer();
    device.destroy(buffer);
    device.destroy(buffer);
    device.destroy(BufferHandle{});
    EXPECT_EQ(device.liveBuffers(), 1u); // retired once, still waiting

    const std::array<std::byte, 1> data{};
    EXPECT_FALSE(device.writeBuffer(buffer, 0, data).has_value());
}

TEST_F(FakeDeviceTest, WaitIdleFinishesSubmittedFrames)
{
    attach();
    const BufferHandle buffer = makeBuffer();
    (void)begin();
    device.destroy(buffer);
    ASSERT_TRUE(device.endFrame().has_value());
    device.waitIdle();
    (void)begin();
    EXPECT_EQ(device.liveBuffers(), 0u);
}

TEST(FakeDevice, DefaultSinkConstructsWithoutDescription)
{
    FakeDevice device;
    const auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
}

TEST(DefaultMessageSink, WritesWarningsAndErrorsToStderrAndDropsInfo)
{
    const MessageSink sink = makeDefaultMessageSink();

    ::testing::internal::CaptureStderr();
    sink(Severity::Info, "info text");
    EXPECT_EQ(::testing::internal::GetCapturedStderr(), "");

    ::testing::internal::CaptureStderr();
    sink(Severity::Warning, "warning text");
    sink(Severity::Error, "error text");
    const std::string output = ::testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("warning text"), std::string::npos);
    EXPECT_NE(output.find("error text"), std::string::npos);
}

TEST(FakeDevice, EmptyMessageSinkFallsBackToTheDefaultSink)
{
    DeviceDescription description;
    description.messageSink = MessageSink{};
    FakeDevice device{std::move(description)};
    ::testing::internal::CaptureStderr();
    const auto result = device.endFrame();
    const std::string output = ::testing::internal::GetCapturedStderr();
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(output.find("endFrame"), std::string::npos);
}
