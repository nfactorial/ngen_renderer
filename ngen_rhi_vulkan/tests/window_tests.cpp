// Spec: rhi-vulkan. Behaviour changes go through it.
// Real Vulkan with validation on and WindowSupport::Expected, presenting to a real SDL window.
#include "command_list.h"
#include "test_support.h"

#include <ngen/platform/sdl_window.h>
#include <ngen/rhi/vulkan/vulkan_device.h>

#include <gtest/gtest.h>

#include <cmath>
#include <optional>
#include <string>

using namespace ngen::rhi;
using namespace ngen::rhi::vulkan;
using ngen::ErrorCode;
using ngen::platform::PixelSize;
using ngen::platform::SdlWindow;
using ngen::platform::WindowEvent;
using test_support::MessageLog;

namespace
{
    // SDL with its real video driver, a 640 x 480 window, and a presenting device with the test pipeline and
    // a triangle's vertex buffer.
    class VulkanDeviceWindowTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
            auto window = SdlWindow::create({"ngen_rhi_vulkan window test", 640, 480});
            ASSERT_TRUE(window.has_value()) << window.error().message;
            window_ = std::move(*window);

            auto device = VulkanDevice::create(DeviceDescription{"ngen_rhi_vulkan_tests", true, log_.sink()},
                                               WindowSupport::Expected);
            ASSERT_TRUE(device.has_value()) << device.error().message;
            device_ = std::move(*device);

            const auto handles = window_->nativeWindow();
            ASSERT_TRUE(handles.has_value()) << handles.error().message;
            const auto attached = device_->attachWindow(*handles, window_->pixelSize());
            ASSERT_TRUE(attached.has_value()) << attached.error().message;

            const auto buffer = device_->createBuffer({1024, BufferUsage::Vertex});
            ASSERT_TRUE(buffer.has_value()) << buffer.error().message;
            buffer_ = *buffer;
            ASSERT_TRUE(device_->writeBuffer(buffer_, 0, test_support::triangleVertices()).has_value());

            const std::vector<std::byte> code = test_support::loadTestShader();
            ASSERT_FALSE(code.empty()) << "the compiled test shader is missing: " << NGEN_TEST_SHADER_PATH;
            const auto shader = device_->createShader({code});
            ASSERT_TRUE(shader.has_value()) << shader.error().message;
            const auto pipeline = device_->createPipeline(test_support::testPipelineDescription(*shader));
            ASSERT_TRUE(pipeline.has_value()) << pipeline.error().message;
            pipeline_ = *pipeline;

            // The pipeline keeps its own copy of the bytecode, so the shader can go at once.
            device_->destroy(*shader);
        }

        void TearDown() override
        {
            device_.reset();
            window_.reset();
            SDL_Quit();
        }

        // Hands every pending SDL event to the window and returns the window events they became.
        std::vector<WindowEvent> pumpEvents() const
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                window_->handleEvent(event);
            }
            std::vector<WindowEvent> events;
            window_->pollEvents(events);
            return events;
        }

        // Presents count frames, each clearing the image and drawing the triangle. With an expected size,
        // each frame must be presented at it (swapChainSize() after its beginFrame).
        ::testing::AssertionResult presentFrames(const int count,
                                                 const std::optional<PixelSize> expectedSize = std::nullopt) const
        {
            for (int i = 0; i < count; ++i)
            {
                (void)pumpEvents();
                const auto frame = device_->beginFrame();
                if (!frame.has_value())
                {
                    return ::testing::AssertionFailure() << "beginFrame failed: " << frame.error().message;
                }
                if (*frame == nullptr)
                {
                    return ::testing::AssertionFailure() << "beginFrame returned no command list for frame " << i;
                }
                if (const PixelSize size = device_->swapChainSize(); expectedSize.has_value() && size != *expectedSize)
                {
                    (void)device_->endFrame();
                    return ::testing::AssertionFailure()
                           << "frame " << i << " presents at " << size.width << " x " << size.height << ", not "
                           << expectedSize->width << " x " << expectedSize->height;
                }
                CommandList &commands = **frame;
                commands.beginRendering(ColorTarget{LoadOp::Clear, StoreOp::Store, {0.1f, 0.2f, 0.3f, 1.0f}});
                commands.bindPipeline(pipeline_);
                commands.bindVertexBuffer(0, buffer_);
                commands.draw(3);
                commands.endRendering();
                if (const auto ended = device_->endFrame(); !ended.has_value())
                {
                    return ::testing::AssertionFailure() << "endFrame failed: " << ended.error().message;
                }
            }
            return ::testing::AssertionSuccess();
        }

        // The window's pixel size for a size in window coordinates.
        PixelSize pixelsFor(const int width, const int height) const
        {
            const float density = SDL_GetWindowPixelDensity(window_->sdlWindow());
            return PixelSize{static_cast<uint32_t>(std::lround(static_cast<float>(width) * density)),
                             static_cast<uint32_t>(std::lround(static_cast<float>(height) * density))};
        }

        MessageLog log_;
        std::unique_ptr<SdlWindow> window_;
        std::unique_ptr<Device> device_;
        BufferHandle buffer_;
        PipelineHandle pipeline_;
    };
} // namespace

// Acceptance (window test): present 3 frames at 640 x 480, resize to 800 x 600 and present 3 at that size,
// suspend (beginFrame returns nullptr), resume and present 3 more, with no Warning or Error.
TEST_F(VulkanDeviceWindowTest, PresentsAcrossAResizeAndASuspension)
{
    ASSERT_TRUE(presentFrames(3, pixelsFor(640, 480)));

    ASSERT_TRUE(SDL_SetWindowSize(window_->sdlWindow(), 800, 600)) << SDL_GetError();
    (void)SDL_SyncWindow(window_->sdlWindow());
    bool resized = false;
    for (const WindowEvent &event : pumpEvents())
    {
        if (event.kind == WindowEvent::Kind::Resized)
        {
            device_->onResized(event.size);
            resized = true;
        }
    }
    ASSERT_TRUE(resized) << "SDL delivered no resize event";
    ASSERT_EQ(window_->pixelSize(), pixelsFor(800, 600));
    ASSERT_TRUE(presentFrames(3, pixelsFor(800, 600)));

    device_->onSuspended();
    const auto suspended = device_->beginFrame();
    ASSERT_TRUE(suspended.has_value()) << suspended.error().message;
    EXPECT_EQ(*suspended, nullptr);
    EXPECT_EQ(device_->swapChainSize(), (PixelSize{0, 0}));

    const auto handles = window_->nativeWindow();
    ASSERT_TRUE(handles.has_value()) << handles.error().message;
    const auto resumed = device_->onResumed(*handles, window_->pixelSize());
    ASSERT_TRUE(resumed.has_value()) << resumed.error().message;
    ASSERT_TRUE(presentFrames(3, pixelsFor(800, 600)));

    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, AZeroSizeHasNoSwapChainUntilTheFirstNonZeroSize)
{
    ASSERT_TRUE(presentFrames(1));

    device_->onResized({0, 0});
    const auto minimised = device_->beginFrame();
    ASSERT_TRUE(minimised.has_value()) << minimised.error().message;
    EXPECT_EQ(*minimised, nullptr);
    EXPECT_EQ(device_->swapChainSize(), (PixelSize{0, 0}));

    device_->onResized(window_->pixelSize());
    ASSERT_TRUE(presentFrames(2));
    EXPECT_EQ(device_->swapChainSize(), window_->pixelSize());

    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, ASecondAttachWindowIsInvalidArgument)
{
    const auto handles = window_->nativeWindow();
    ASSERT_TRUE(handles.has_value());
    const auto again = device_->attachWindow(*handles, window_->pixelSize());
    ASSERT_FALSE(again.has_value());
    EXPECT_EQ(again.error().code, ErrorCode::InvalidArgument);
}

TEST_F(VulkanDeviceWindowTest, AWindowKindThePlatformDoesNotUseIsUnsupported)
{
    // A device holds one swap chain, so a fresh device is attached to the wrong kind of window.
    const MessageLog log;
    auto device = VulkanDevice::create(DeviceDescription{"ngen_rhi_vulkan_tests", true, log.sink()},
                                       WindowSupport::Expected);
    ASSERT_TRUE(device.has_value()) << device.error().message;
#if defined(_WIN32)
    const ngen::platform::NativeWindow wrongKind = ngen::platform::AppleHandles{nullptr};
#else
    const ngen::platform::NativeWindow wrongKind = ngen::platform::Win32Handles{nullptr, nullptr};
#endif
    const auto attached = (*device)->attachWindow(wrongKind, {640, 480});
    ASSERT_FALSE(attached.has_value());
    EXPECT_EQ(attached.error().code, ErrorCode::Unsupported);
    EXPECT_EQ((*device)->beginFrame().value_or(nullptr), nullptr);
    device->reset();
    EXPECT_EQ(log.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, ResourcesDestroyedDuringAFrameLiveUntilItFinishes)
{
    // A second buffer and pipeline, used by a frame and destroyed while it is recorded: validation reports
    // any destruction while the GPU still uses them.
    const auto buffer = device_->createBuffer({256, BufferUsage::Vertex});
    ASSERT_TRUE(buffer.has_value());
    ASSERT_TRUE(device_->writeBuffer(*buffer, 0, test_support::triangleVertices()).has_value());
    const std::vector<std::byte> code = test_support::loadTestShader();
    const auto shader = device_->createShader({code});
    ASSERT_TRUE(shader.has_value());
    const auto pipeline = device_->createPipeline(test_support::testPipelineDescription(*shader));
    ASSERT_TRUE(pipeline.has_value());

    const auto frame = device_->beginFrame();
    ASSERT_TRUE(frame.has_value() && *frame != nullptr);
    CommandList &commands = **frame;
    commands.beginRendering(ColorTarget{});
    commands.bindPipeline(*pipeline);
    commands.bindVertexBuffer(0, *buffer);
    device_->destroy(*pipeline);
    device_->destroy(*buffer);
    device_->destroy(*shader);
    commands.draw(3);
    commands.endRendering();
    ASSERT_TRUE(device_->endFrame().has_value());

    // The handles are stale at once.
    const auto written = device_->writeBuffer(*buffer, 0, test_support::triangleVertices());
    ASSERT_FALSE(written.has_value());
    EXPECT_EQ(written.error().code, ErrorCode::InvalidArgument);

    ASSERT_TRUE(presentFrames(4));
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, MisuseIsSkippedWithOneErrorMessageEach)
{
    const auto uniform = device_->createBuffer({256, BufferUsage::Uniform});
    ASSERT_TRUE(uniform.has_value());
    const auto stale = device_->createBuffer({256, BufferUsage::Vertex});
    ASSERT_TRUE(stale.has_value());
    device_->destroy(*stale);

    const auto frame = device_->beginFrame();
    ASSERT_TRUE(frame.has_value() && *frame != nullptr);
    CommandList &commands = **frame;

    // Each step is misuse that must be skipped with exactly one Error message (and nothing from validation).
    const auto expectOneError = [this](const char *const what) {
        ASSERT_EQ(log_.messages().size(), 1u) << what << ":\n" << log_.problems();
        EXPECT_EQ(log_.messages()[0].severity, Severity::Error) << what;
        log_.clear();
    };

    const auto again = device_->beginFrame();
    ASSERT_FALSE(again.has_value());
    EXPECT_EQ(again.error().code, ErrorCode::InvalidArgument);
    expectOneError("beginFrame while recording");

    commands.draw(3);
    expectOneError("draw outside rendering");
    commands.bindPipeline(pipeline_);
    expectOneError("bindPipeline outside rendering");
    commands.bindVertexBuffer(0, buffer_);
    expectOneError("bindVertexBuffer outside rendering");
    commands.endRendering();
    expectOneError("endRendering while not rendering");

    // Viewport and scissor may be set anywhere in a frame.
    commands.setViewport(0, 0, 100, 100);
    commands.setScissor(0, 0, 100, 100);
    EXPECT_TRUE(log_.messages().empty()) << log_.problems();

    commands.beginRendering(ColorTarget{});
    commands.beginRendering(ColorTarget{});
    expectOneError("beginRendering while rendering");
    commands.draw(3);
    expectOneError("draw with no pipeline bound");
    commands.bindPipeline(PipelineHandle{});
    expectOneError("bindPipeline with a null handle");
    commands.bindPipeline(pipeline_);
    commands.draw(3);
    expectOneError("draw with the pipeline's binding unbound");
    commands.bindVertexBuffer(0, *uniform);
    expectOneError("bindVertexBuffer without BufferUsage::Vertex");
    commands.bindVertexBuffer(0, buffer_, 1024);
    expectOneError("bindVertexBuffer at an offset past the buffer");
    commands.bindVertexBuffer(0, *stale);
    expectOneError("bindVertexBuffer with a stale handle");
    commands.bindVertexBuffer(0, buffer_);
    commands.draw(3);
    EXPECT_TRUE(log_.messages().empty()) << log_.problems();

    // Rendering left open is reported, and the frame is still submitted.
    const auto ended = device_->endFrame();
    ASSERT_TRUE(ended.has_value()) << ended.error().message;
    expectOneError("endFrame with rendering open");

    commands.draw(3);
    expectOneError("draw after endFrame");

    ASSERT_TRUE(presentFrames(3));
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, LoadAndFramesWithoutRenderingPresentCleanly)
{
    // A first pass loading the image while it is Undefined, then a second pass loading the first's result.
    for (int i = 0; i < 4; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        CommandList &commands = **frame;
        commands.beginRendering(ColorTarget{LoadOp::Load, StoreOp::Store});
        commands.endRendering();
        commands.beginRendering(ColorTarget{LoadOp::Load, StoreOp::DontCare});
        commands.bindPipeline(pipeline_);
        commands.bindVertexBuffer(0, buffer_);
        commands.draw(3);
        commands.endRendering();
        ASSERT_TRUE(device_->endFrame().has_value());
    }

    // A new swap chain's fresh images, presented by frames that record no pass at all.
    device_->onResized({0, 0});
    ASSERT_EQ(device_->beginFrame().value_or(nullptr), nullptr);
    device_->onResized(window_->pixelSize());
    for (int i = 0; i < 4; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        ASSERT_TRUE(device_->endFrame().has_value());
    }
    ASSERT_TRUE(presentFrames(2));
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, TheDestructorFreesResourcesRetiredByFramesStillInFlight)
{
    const auto buffer = device_->createBuffer({256, BufferUsage::Vertex});
    ASSERT_TRUE(buffer.has_value());
    ASSERT_TRUE(device_->writeBuffer(*buffer, 0, test_support::triangleVertices()).has_value());

    const auto frame = device_->beginFrame();
    ASSERT_TRUE(frame.has_value() && *frame != nullptr);
    CommandList &commands = **frame;
    commands.beginRendering(ColorTarget{});
    commands.bindPipeline(pipeline_);
    commands.bindVertexBuffer(0, *buffer);
    commands.draw(3);
    commands.endRendering();
    device_->destroy(*buffer);
    device_->destroy(pipeline_);
    ASSERT_TRUE(device_->endFrame().has_value());

    // Validation reports any object the device leaves behind, and VMA any allocation.
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, SuspendingWhileRecordingTakesEffectAfterEndFrame)
{
    const auto frame = device_->beginFrame();
    ASSERT_TRUE(frame.has_value() && *frame != nullptr);
    (*frame)->beginRendering(ColorTarget{});
    device_->onSuspended();
    (*frame)->bindPipeline(pipeline_);
    (*frame)->bindVertexBuffer(0, buffer_);
    (*frame)->draw(3);
    (*frame)->endRendering();

    // The surface is replaced only between frames.
    const auto handles = window_->nativeWindow();
    ASSERT_TRUE(handles.has_value());
    const auto early = device_->onResumed(*handles, window_->pixelSize());
    ASSERT_FALSE(early.has_value());
    EXPECT_EQ(early.error().code, ErrorCode::InvalidArgument);
    ASSERT_EQ(log_.messages().size(), 1u) << log_.problems();
    log_.clear();

    ASSERT_TRUE(device_->endFrame().has_value());
    const auto suspended = device_->beginFrame();
    ASSERT_TRUE(suspended.has_value());
    EXPECT_EQ(*suspended, nullptr);
    EXPECT_EQ(device_->swapChainSize(), (PixelSize{0, 0}));

    ASSERT_TRUE(device_->onResumed(*handles, window_->pixelSize()).has_value());
    ASSERT_TRUE(presentFrames(2));
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, EachPassStartsWithTheFullImageAsViewportAndScissor)
{
    const auto frame = device_->beginFrame();
    ASSERT_TRUE(frame.has_value() && *frame != nullptr);
    CommandList &commands = **frame;
    const auto &recorded = static_cast<const VulkanCommandList &>(commands);
    const PixelSize size = device_->swapChainSize();
    const auto expectFullImage = [&](const char *const when) {
        EXPECT_EQ(recorded.viewport().x, 0.0f) << when;
        EXPECT_EQ(recorded.viewport().y, 0.0f) << when;
        EXPECT_EQ(recorded.viewport().width, static_cast<float>(size.width)) << when;
        EXPECT_EQ(recorded.viewport().height, static_cast<float>(size.height)) << when;
        EXPECT_EQ(recorded.viewport().minDepth, 0.0f) << when;
        EXPECT_EQ(recorded.viewport().maxDepth, 1.0f) << when;
        EXPECT_EQ(recorded.scissor().offset.x, 0) << when;
        EXPECT_EQ(recorded.scissor().offset.y, 0) << when;
        EXPECT_EQ(recorded.scissor().extent.width, size.width) << when;
        EXPECT_EQ(recorded.scissor().extent.height, size.height) << when;
    };

    // Set before the first pass: allowed, and overridden when the pass begins.
    commands.setViewport(1, 2, 30, 40, 0.25f, 0.75f);
    commands.setScissor(3, 4, 50, 60);
    commands.beginRendering(ColorTarget{});
    expectFullImage("at the first beginRendering");

    // Set inside the first pass: it applies there...
    commands.setViewport(10, 10, 50, 50, 0.5f, 0.5f);
    commands.setScissor(5, 5, 20, 20);
    EXPECT_EQ(recorded.viewport().width, 50.0f);
    EXPECT_EQ(recorded.scissor().extent.width, 20u);
    commands.bindPipeline(pipeline_);
    commands.bindVertexBuffer(0, buffer_);
    commands.draw(3);
    commands.endRendering();

    // ...and not in the next pass.
    commands.beginRendering(ColorTarget{LoadOp::Load, StoreOp::Store});
    expectFullImage("at the second beginRendering");
    commands.bindPipeline(pipeline_);
    commands.bindVertexBuffer(0, buffer_);
    commands.draw(3);
    commands.endRendering();
    ASSERT_TRUE(device_->endFrame().has_value());

    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

namespace
{
    Barrier swapChainBarrier(const ResourceState before, const ResourceState after)
    {
        return Barrier{SwapChainImage{}, before, after};
    }
} // namespace

// Acceptance (window test): explicit barriers around a Clear pass and a Load pass, a pass with no barrier first,
// and a frame with no pass each present with no Warning or Error. Each kind runs for more frames than the swap
// chain has images, so every image is used by it.
TEST_F(VulkanDeviceWindowTest, ExplicitBarriersImplicitTransitionsAndFramesWithNoPassPresentCleanly)
{
    constexpr int kFrames = 6;
    for (int i = 0; i < kFrames; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        CommandList &commands = **frame;
        commands.barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment));
        commands.beginRendering(ColorTarget{LoadOp::Clear, StoreOp::Store, {0.1f, 0.2f, 0.3f, 1.0f}});
        commands.endRendering();
        commands.barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::ColorAttachment));
        commands.beginRendering(ColorTarget{LoadOp::Load, StoreOp::Store});
        commands.bindPipeline(pipeline_);
        commands.bindVertexBuffer(0, buffer_);
        commands.draw(3);
        commands.endRendering();
        ASSERT_TRUE(device_->endFrame().has_value());
        // The two barriers, and endFrame's move to Present; beginRendering found the image a colour attachment.
        EXPECT_EQ(static_cast<const VulkanCommandList &>(commands).pipelineBarrierCount(), 3u);
    }
    for (int i = 0; i < kFrames; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        CommandList &commands = **frame;
        commands.beginRendering(ColorTarget{});
        commands.bindPipeline(pipeline_);
        commands.bindVertexBuffer(0, buffer_);
        commands.draw(3);
        commands.endRendering();
        ASSERT_TRUE(device_->endFrame().has_value());
        // beginRendering's own move to ColorAttachment, and endFrame's move to Present.
        EXPECT_EQ(static_cast<const VulkanCommandList &>(commands).pipelineBarrierCount(), 2u);
    }
    for (int i = 0; i < kFrames; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        ASSERT_TRUE(device_->endFrame().has_value());
        // Only endFrame's move to Present, from Undefined.
        EXPECT_EQ(static_cast<const VulkanCommandList &>(**frame).pipelineBarrierCount(), 1u);
    }
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

// Acceptance (window test): in a later frame than the first, a Load pass recorded while the image is Undefined
// (with no barrier first, as the frame's first pass) presents with no Warning or Error.
TEST_F(VulkanDeviceWindowTest, ALoadPassWhileTheImageIsUndefinedInALaterFramePresentsCleanly)
{
    ASSERT_TRUE(presentFrames(3));
    for (int i = 0; i < 6; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        CommandList &commands = **frame;
        commands.beginRendering(ColorTarget{LoadOp::Load, StoreOp::Store});
        commands.bindPipeline(pipeline_);
        commands.bindVertexBuffer(0, buffer_);
        commands.draw(3);
        commands.endRendering();
        ASSERT_TRUE(device_->endFrame().has_value());
    }
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

// endFrame moves the image to Present only when it is not already there.
TEST_F(VulkanDeviceWindowTest, AnImageAlreadyPresentIsNotMovedAgainByEndFrame)
{
    for (int i = 0; i < 6; ++i)
    {
        const auto frame = device_->beginFrame();
        ASSERT_TRUE(frame.has_value() && *frame != nullptr);
        CommandList &commands = **frame;
        if (i % 2 == 0)
        {
            commands.barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::Present));
            ASSERT_TRUE(device_->endFrame().has_value());
            EXPECT_EQ(static_cast<const VulkanCommandList &>(commands).pipelineBarrierCount(), 1u);
        }
        else
        {
            commands.beginRendering(ColorTarget{});
            commands.bindPipeline(pipeline_);
            commands.bindVertexBuffer(0, buffer_);
            commands.draw(3);
            commands.endRendering();
            commands.barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::Present));
            ASSERT_TRUE(device_->endFrame().has_value());
            EXPECT_EQ(static_cast<const VulkanCommandList &>(commands).pipelineBarrierCount(), 2u);
        }
    }
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}

TEST_F(VulkanDeviceWindowTest, BarrierMisuseIsSkippedWithOneErrorMessageEach)
{
    const auto frame = device_->beginFrame();
    ASSERT_TRUE(frame.has_value() && *frame != nullptr);
    CommandList &commands = **frame;
    const auto &recorded = static_cast<const VulkanCommandList &>(commands);

    const auto expectOneError = [this](const char *const what) {
        ASSERT_EQ(log_.messages().size(), 1u) << what << ":\n" << log_.problems();
        EXPECT_EQ(log_.messages()[0].severity, Severity::Error) << what;
        log_.clear();
    };

    commands.barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::Present));
    ASSERT_EQ(log_.messages().size(), 1u) << log_.problems();
    EXPECT_NE(log_.messages()[0].text.find("Undefined"), std::string::npos) << log_.messages()[0].text;
    EXPECT_NE(log_.messages()[0].text.find("ColorAttachment"), std::string::npos) << log_.messages()[0].text;
    expectOneError("barrier from a state the image is not in");
    commands.barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::Undefined));
    expectOneError("barrier to Undefined");
    commands.barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ShaderRead));
    expectOneError("barrier moving the swap chain image to ShaderRead");
    EXPECT_EQ(recorded.pipelineBarrierCount(), 0u);

    commands.beginRendering(ColorTarget{});
    commands.barrier(swapChainBarrier(ResourceState::ColorAttachment, ResourceState::ColorAttachment));
    expectOneError("barrier inside a pass");
    EXPECT_EQ(recorded.pipelineBarrierCount(), 1u); // beginRendering's own
    commands.endRendering();
    ASSERT_TRUE(device_->endFrame().has_value());

    commands.barrier(swapChainBarrier(ResourceState::Undefined, ResourceState::ColorAttachment));
    expectOneError("barrier after endFrame");

    ASSERT_TRUE(presentFrames(3));
    device_.reset();
    EXPECT_EQ(log_.problems(), "");
}
