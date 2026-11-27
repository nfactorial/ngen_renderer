// Spec: renderer. Behaviour changes go through it.
#include <ngen/renderer/renderer.h>
#include <ngen/rhi/fake_device.h>

#include <gtest/gtest.h>

#include <array>
#include <string_view>
#include <string>
#include <utility>
#include <vector>

using namespace ngen::renderer;
using namespace ngen::rhi;
using ngen::platform::NativeWindow;
using ngen::platform::PixelSize;
using ngen::platform::Win32Handles;

namespace
{
    const std::array<std::byte, 4> kBytecode{};

    class RendererTest : public ::testing::Test
    {
    protected:
        RendererTest()
            : device([this] {
                  DeviceDescription description;
                  description.messageSink = [this](const Severity severity, const std::string_view text) {
                      if (severity != Severity::Info)
                      {
                          problems.emplace_back(text);
                      }
                  };
                  return FakeDevice(std::move(description));
              }())
        {
            EXPECT_TRUE(device.attachWindow(NativeWindow{Win32Handles{}}, PixelSize{640, 480}).has_value());
            auto created = Renderer::create(device, RendererConfiguration{});
            EXPECT_TRUE(created.has_value());
            renderer = std::move(*created);
        }

        // Nothing the renderer does should make the device report a warning or error, unless the test provoked
        // one itself by misusing the device.
        void TearDown() override
        {
            EXPECT_EQ(problems.size(), expectedProblems) << (problems.empty() ? "" : problems.front());
        }

        PipelineHandle makePipeline()
        {
            GraphicsPipelineDescription description;
            description.vertex = {device.createShader(ShaderDescription{kBytecode}).value(), "main"};
            description.fragment = {device.createShader(ShaderDescription{kBytecode}).value(), "main"};
            description.bindings = {{0, 12}};
            description.attributes = {{0, 0, Format::R32G32B32Float, 0}};
            return device.createPipeline(description).value();
        }

        BufferHandle makeBuffer()
        {
            return device.createBuffer(BufferDescription{64, BufferUsage::Vertex}).value();
        }

        DrawItem item(const char *const stageName, const PipelineHandle pipeline, const BufferHandle buffer,
                      const uint32_t vertexCount)
        {
            DrawItem draw;
            draw.stage = renderer->stage(stageName).value();
            draw.pipeline = pipeline;
            draw.vertexBuffers = {{0, buffer}};
            draw.vertexCount = vertexCount;
            return draw;
        }

        const std::vector<RecordedCommand> &onlyFrame() const
        {
            EXPECT_EQ(device.frames().size(), 1u);
            return device.frames().front();
        }

        std::vector<std::string> problems;
        size_t expectedProblems = 0;
        FakeDevice device;
        std::unique_ptr<Renderer> renderer;
    };


    // Forwards to a FakeDevice, except that endFrame fails.
    class FailingEndFrameDevice final : public Device
    {
    public:
        explicit FailingEndFrameDevice(FakeDevice &inner) : inner_(inner) {}

        std::expected<BufferHandle, ngen::Error> createBuffer(const BufferDescription &d) override
        {
            return inner_.createBuffer(d);
        }
        std::expected<void, ngen::Error> writeBuffer(const BufferHandle b, const uint64_t offset,
                                                     const std::span<const std::byte> data) override
        {
            return inner_.writeBuffer(b, offset, data);
        }
        std::expected<ShaderHandle, ngen::Error> createShader(const ShaderDescription &d) override
        {
            return inner_.createShader(d);
        }
        std::expected<PipelineHandle, ngen::Error> createPipeline(const GraphicsPipelineDescription &d) override
        {
            return inner_.createPipeline(d);
        }
        void destroy(const BufferHandle h) override { inner_.destroy(h); }
        void destroy(const ShaderHandle h) override { inner_.destroy(h); }
        void destroy(const PipelineHandle h) override { inner_.destroy(h); }
        std::expected<void, ngen::Error> attachWindow(const NativeWindow &window, const PixelSize size) override
        {
            return inner_.attachWindow(window, size);
        }
        void onResized(const PixelSize size) override { inner_.onResized(size); }
        void onSuspended() override { inner_.onSuspended(); }
        std::expected<void, ngen::Error> onResumed(const NativeWindow &window, const PixelSize size) override
        {
            return inner_.onResumed(window, size);
        }
        std::expected<CommandList *, ngen::Error> beginFrame() override { return inner_.beginFrame(); }
        std::expected<void, ngen::Error> endFrame() override
        {
            return std::unexpected(ngen::makeError(ngen::ErrorCode::DeviceLost, "endFrame failed for the test"));
        }
        PixelSize swapChainSize() const override { return inner_.swapChainSize(); }
        void waitIdle() override { inner_.waitIdle(); }

    private:
        FakeDevice &inner_;
    };

    constexpr Barrier kInitial{SwapChainImage{}, ResourceState::Undefined, ResourceState::ColorAttachment};
    constexpr Barrier kBetween{SwapChainImage{}, ResourceState::ColorAttachment, ResourceState::ColorAttachment};
    const ColorTarget kClear{LoadOp::Clear, StoreOp::Store, {0, 0, 0, 1}};
    const ColorTarget kLoad{LoadOp::Load, StoreOp::Store, {0, 0, 0, 1}};

    bool mentions(const ngen::Error &error, const std::string &text)
    {
        return error.message.find(text) != std::string::npos;
    }
} // namespace

TEST(RendererConfigurationTest, DefaultStagesListShadowOpaqueTransparentPostProcessUiInOrder)
{
    const std::vector<StageDescription> stages = defaultStages();
    ASSERT_EQ(stages.size(), 5u);
    EXPECT_EQ(stages[0].name, "Shadow");
    EXPECT_EQ(stages[1].name, "Opaque");
    EXPECT_EQ(stages[2].name, "Transparent");
    EXPECT_EQ(stages[3].name, "PostProcess");
    EXPECT_EQ(stages[4].name, "UI");
}

TEST(RendererConfigurationTest, DefaultConfigurationIsForwardBlackOnDefaultStagesAndValid)
{
    const RendererConfiguration configuration;
    EXPECT_EQ(configuration.lighting, LightingPath::Forward);
    EXPECT_EQ(configuration.clearColor, (std::array<float, 4>{0, 0, 0, 1}));
    EXPECT_EQ(configuration.stages.size(), 5u);
    EXPECT_TRUE(validate(configuration).has_value());
}

TEST(RendererConfigurationTest, ValidateReturnsInvalidArgumentWhenThereAreNoStages)
{
    RendererConfiguration configuration;
    configuration.stages.clear();
    const auto result = validate(configuration);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_TRUE(mentions(result.error(), "no stages")) << result.error().message;
}

TEST(RendererConfigurationTest, ValidateNamesTheStageWhenANameIsUsedTwice)
{
    RendererConfiguration configuration;
    configuration.stages = {{"Opaque"}, {"UI"}, {"Opaque"}};
    const auto result = validate(configuration);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_TRUE(mentions(result.error(), "Opaque")) << result.error().message;
}

TEST(RendererConfigurationTest, ValidateNamesTheStageWhenItsNameIsEmpty)
{
    RendererConfiguration configuration;
    configuration.stages = {{"Opaque"}, {""}};
    const auto result = validate(configuration);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_TRUE(mentions(result.error(), "stage 1")) << result.error().message;
}

TEST_F(RendererTest, CreateReturnsInvalidArgumentWhenConfigurationIsInvalid)
{
    RendererConfiguration none;
    none.stages.clear();
    const auto noStages = Renderer::create(device, none);
    ASSERT_FALSE(noStages.has_value());
    EXPECT_EQ(noStages.error().code, ngen::ErrorCode::InvalidArgument);

    RendererConfiguration twice;
    twice.stages = {{"Opaque"}, {"Opaque"}};
    const auto duplicate = Renderer::create(device, twice);
    ASSERT_FALSE(duplicate.has_value());
    EXPECT_EQ(duplicate.error().code, ngen::ErrorCode::InvalidArgument);
}

TEST_F(RendererTest, ConfigurationReturnsWhatCreateWasGiven)
{
    RendererConfiguration custom;
    custom.stages = {{"A"}, {"B"}};
    custom.clearColor = {1, 0.5f, 0.25f, 1};
    auto created = Renderer::create(device, custom);
    ASSERT_TRUE(created.has_value());
    EXPECT_EQ((*created)->configuration().clearColor, custom.clearColor);
    EXPECT_EQ((*created)->configuration().stages.size(), 2u);
}

TEST_F(RendererTest, StageFindsConfiguredNamesAndNotMissingOnes)
{
    EXPECT_EQ(renderer->stage("Opaque"), std::optional<StageId>(StageId{1}));
    EXPECT_FALSE(renderer->stage("Missing").has_value());
}

TEST_F(RendererTest, RenderRecordsStagesInConfigurationOrderWhenListOrderDiffers)
{
    const PipelineHandle p = makePipeline();
    const PipelineHandle q = makePipeline();
    const BufferHandle b = makeBuffer();
    const BufferHandle c = makeBuffer();

    RenderList list;
    list.add(item("UI", p, b, 3));
    list.add(item("Opaque", q, c, 6));

    const auto result = renderer->render(list);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, FrameResult::Presented);

    const std::vector<RecordedCommand> expected{
        kInitial,
        BeginRenderingCommand{kClear},
        BindPipelineCommand{q},
        BindVertexBufferCommand{0, c, 0},
        DrawCommand{6, 1, 0, 0},
        EndRenderingCommand{},
        kBetween,
        BeginRenderingCommand{kLoad},
        BindPipelineCommand{p},
        BindVertexBufferCommand{0, b, 0},
        DrawCommand{3, 1, 0, 0},
        EndRenderingCommand{},
    };
    EXPECT_EQ(onlyFrame(), expected);
}

TEST_F(RendererTest, RenderBindsPipelineOnceWhenConsecutiveItemsShareIt)
{
    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();
    const BufferHandle c = makeBuffer();

    RenderList list;
    list.add(item("Opaque", p, b, 3));
    list.add(item("Opaque", p, c, 6));
    ASSERT_TRUE(renderer->render(list).has_value());

    const std::vector<RecordedCommand> expected{
        kInitial,
        BeginRenderingCommand{kClear},
        BindPipelineCommand{p},
        BindVertexBufferCommand{0, b, 0},
        DrawCommand{3, 1, 0, 0},
        BindVertexBufferCommand{0, c, 0},
        DrawCommand{6, 1, 0, 0},
        EndRenderingCommand{},
    };
    EXPECT_EQ(onlyFrame(), expected);
}

TEST_F(RendererTest, RenderBindsAgainWhenThePipelineChangesWithinAStage)
{
    const PipelineHandle p = makePipeline();
    const PipelineHandle q = makePipeline();
    const BufferHandle b = makeBuffer();

    RenderList list;
    list.add(item("Opaque", p, b, 3));
    list.add(item("Opaque", q, b, 3));
    list.add(item("Opaque", p, b, 3));
    ASSERT_TRUE(renderer->render(list).has_value());

    size_t binds = 0;
    for (const RecordedCommand &command : onlyFrame())
    {
        binds += std::holds_alternative<BindPipelineCommand>(command) ? 1 : 0;
    }
    EXPECT_EQ(binds, 3u);
}

TEST_F(RendererTest, RenderBindsAgainWhenTheSamePipelineStartsANewStage)
{
    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();

    RenderList list;
    list.add(item("Opaque", p, b, 3));
    list.add(item("UI", p, b, 3));
    ASSERT_TRUE(renderer->render(list).has_value());

    size_t binds = 0;
    for (const RecordedCommand &command : onlyFrame())
    {
        binds += std::holds_alternative<BindPipelineCommand>(command) ? 1 : 0;
    }
    EXPECT_EQ(binds, 2u);
}

TEST_F(RendererTest, RenderPassesDrawArgumentsAndBufferOffsetsThrough)
{
    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();
    const BufferHandle c = makeBuffer();

    DrawItem draw = item("Opaque", p, b, 9);
    draw.vertexBuffers = {{0, b, 16}, {1, c, 8}};
    draw.instanceCount = 4;
    draw.firstVertex = 2;
    draw.firstInstance = 3;
    RenderList list;
    list.add(draw);
    ASSERT_TRUE(renderer->render(list).has_value());

    const std::vector<RecordedCommand> expected{
        kInitial,
        BeginRenderingCommand{kClear},
        BindPipelineCommand{p},
        BindVertexBufferCommand{0, b, 16},
        BindVertexBufferCommand{1, c, 8},
        DrawCommand{9, 4, 2, 3},
        EndRenderingCommand{},
    };
    EXPECT_EQ(onlyFrame(), expected);
}

TEST_F(RendererTest, RenderClearsToConfiguredColorOnTheFirstPassOnly)
{
    RendererConfiguration custom;
    custom.clearColor = {0.1f, 0.2f, 0.3f, 1};
    auto created = Renderer::create(device, custom);
    ASSERT_TRUE(created.has_value());
    renderer = std::move(*created);

    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();
    RenderList list;
    list.add(item("Opaque", p, b, 3));
    list.add(item("UI", p, b, 3));
    ASSERT_TRUE(renderer->render(list).has_value());

    std::vector<ColorTarget> targets;
    for (const RecordedCommand &command : onlyFrame())
    {
        if (const auto *begin = std::get_if<BeginRenderingCommand>(&command))
        {
            targets.push_back(begin->target);
        }
    }
    ASSERT_EQ(targets.size(), 2u);
    EXPECT_EQ(targets[0].load, LoadOp::Clear);
    EXPECT_EQ(targets[0].clearColor, custom.clearColor);
    EXPECT_EQ(targets[1].load, LoadOp::Load);
}

TEST_F(RendererTest, RenderClearsWhenTheListIsEmpty)
{
    const auto result = renderer->render(RenderList{});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, FrameResult::Presented);

    const std::vector<RecordedCommand> expected{kInitial, BeginRenderingCommand{kClear}, EndRenderingCommand{}};
    EXPECT_EQ(onlyFrame(), expected);
}

TEST_F(RendererTest, RenderClearsToConfiguredColorWhenTheListIsEmpty)
{
    RendererConfiguration custom;
    custom.clearColor = {0.1f, 0.2f, 0.3f, 1};
    auto created = Renderer::create(device, custom);
    ASSERT_TRUE(created.has_value());
    ASSERT_TRUE((*created)->render(RenderList{}).has_value());
    ASSERT_EQ(device.frames().size(), 1u);
    const auto *begin = std::get_if<BeginRenderingCommand>(&device.frames().front()[1]);
    ASSERT_NE(begin, nullptr);
    EXPECT_EQ(begin->target.clearColor, custom.clearColor);
}

TEST_F(RendererTest, RenderSkipsWhenTheDeviceIsSuspended)
{
    device.onSuspended();
    const auto result = renderer->render(RenderList{});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, FrameResult::Skipped);
    EXPECT_TRUE(device.frames().empty());
}

TEST_F(RendererTest, RenderNamesTheItemAndBeginsNoFrameWhenItsStageIsUnknown)
{
    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();

    RenderList list;
    DrawItem bad = item("Opaque", p, b, 3);
    bad.stage = StageId{9};
    list.add(bad);

    const auto result = renderer->render(list);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_TRUE(mentions(result.error(), "item 0")) << result.error().message;
    EXPECT_TRUE(device.frames().empty());

    // No frame was left begun: a valid list renders afterwards.
    EXPECT_EQ(renderer->render(RenderList{}).value_or(FrameResult::Skipped), FrameResult::Presented);
}

TEST_F(RendererTest, RenderNamesTheItemAndBeginsNoFrameWhenItsVertexCountIsZero)
{
    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();

    RenderList list;
    list.add(item("Opaque", p, b, 3));
    list.add(item("UI", p, b, 0));

    const auto result = renderer->render(list);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_TRUE(mentions(result.error(), "item 1")) << result.error().message;
    EXPECT_TRUE(device.frames().empty());
    EXPECT_EQ(renderer->render(RenderList{}).value_or(FrameResult::Skipped), FrameResult::Presented);
}

TEST_F(RendererTest, RenderRecordsTheSameFrameTwiceWhenGivenTheSameListTwice)
{
    const PipelineHandle p = makePipeline();
    const BufferHandle b = makeBuffer();
    RenderList list;
    list.add(item("Opaque", p, b, 3));

    ASSERT_TRUE(renderer->render(list).has_value());
    ASSERT_TRUE(renderer->render(list).has_value());
    ASSERT_EQ(device.frames().size(), 2u);
    EXPECT_EQ(device.frames()[0], device.frames()[1]);
}

TEST_F(RendererTest, RenderReturnsBeginFrameErrorUnchangedWhenAFrameIsAlreadyOpen)
{
    ASSERT_TRUE(device.beginFrame().has_value());
    expectedProblems = 1; // the device reports the second beginFrame

    const auto result = renderer->render(RenderList{});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    EXPECT_EQ(result.error().message, "beginFrame: a frame is already being recorded");

    ASSERT_TRUE(device.endFrame().has_value());
}

TEST_F(RendererTest, RenderReturnsEndFrameErrorUnchangedWhenEndFrameFails)
{
    FailingEndFrameDevice failing(device);
    auto created = Renderer::create(failing, RendererConfiguration{});
    ASSERT_TRUE(created.has_value());

    const auto result = (*created)->render(RenderList{});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::DeviceLost);
    EXPECT_EQ(result.error().message, "endFrame failed for the test");
}
