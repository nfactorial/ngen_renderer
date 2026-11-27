// Spec: render-graph. Behaviour changes go through it.
#include <ngen/renderer/render_graph.h>
#include <ngen/rhi/fake_device.h>

#include <gtest/gtest.h>

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace ngen::renderer;
using namespace ngen::rhi;
using ngen::platform::NativeWindow;
using ngen::platform::Win32Handles;

namespace
{
    constexpr std::array<float, 4> kBlack{0, 0, 0, 1};

    PassDescription pass(std::string name, const int32_t order, const ResourceId target,
                         const std::optional<std::array<float, 4>> clear = std::nullopt)
    {
        PassDescription description;
        description.name = std::move(name);
        description.order = order;
        description.color = {target, clear};
        return description;
    }

    bool mentions(const ngen::Error &error, const std::string &text)
    {
        return error.message.find(text) != std::string::npos;
    }

    void expectInvalidNaming(const RenderGraph &graph, const std::string &name, const std::string &reason = "")
    {
        const auto result = graph.compile();
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
        EXPECT_TRUE(mentions(result.error(), name)) << result.error().message;
        EXPECT_TRUE(mentions(result.error(), reason)) << result.error().message;
    }

    // The first acceptance's graph: "a" (order 2) and "b" (order 1, cleared) both write the swap chain image.
    RenderGraph firstGraph(std::vector<std::string> *const log = nullptr)
    {
        RenderGraph graph;
        const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
        PassDescription a = pass("a", 2, image);
        PassDescription b = pass("b", 1, image, kBlack);
        a.record = [log](CommandList &commands) {
            if (log)
            {
                log->push_back("a");
            }
            commands.draw(1);
        };
        b.record = [log](CommandList &commands) {
            if (log)
            {
                log->push_back("b");
            }
            commands.draw(2);
        };
        graph.addPass(std::move(a));
        graph.addPass(std::move(b));
        return graph;
    }
} // namespace

TEST(RenderGraphTest, OrdersByOrderKeyWithBarriersAndLoadOps)
{
    const auto compiled = firstGraph().compile();
    ASSERT_TRUE(compiled.has_value());
    ASSERT_EQ(compiled->passes.size(), 2u);

    const CompiledPass &b = compiled->passes[0];
    EXPECT_EQ(b.name, "b");
    EXPECT_EQ(b.barriers,
              (std::vector<Barrier>{{SwapChainImage{}, ResourceState::Undefined, ResourceState::ColorAttachment}}));
    EXPECT_EQ(b.colorTarget, (ColorTarget{LoadOp::Clear, StoreOp::Store, kBlack}));

    const CompiledPass &a = compiled->passes[1];
    EXPECT_EQ(a.name, "a");
    EXPECT_EQ(a.barriers, (std::vector<Barrier>{
                              {SwapChainImage{}, ResourceState::ColorAttachment, ResourceState::ColorAttachment}}));
    EXPECT_EQ(a.colorTarget.load, LoadOp::Load);
    EXPECT_EQ(a.colorTarget.store, StoreOp::Store);
}

TEST(RenderGraphTest, EqualOrderRunsInOrderAdded)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    graph.addPass(pass("third", 5, image));
    graph.addPass(pass("first", 0, image));
    graph.addPass(pass("second", 0, image));
    graph.addPass(pass("fourth", 5, image));

    const auto compiled = graph.compile();
    ASSERT_TRUE(compiled.has_value());
    ASSERT_EQ(compiled->passes.size(), 4u);
    EXPECT_EQ(compiled->passes[0].name, "first");
    EXPECT_EQ(compiled->passes[1].name, "second");
    EXPECT_EQ(compiled->passes[2].name, "third");
    EXPECT_EQ(compiled->passes[3].name, "fourth");
}

TEST(RenderGraphTest, WritingUndefinedImportWithoutClearIsDontCare)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    graph.addPass(pass("p", 0, image));

    const auto compiled = graph.compile();
    ASSERT_TRUE(compiled.has_value());
    EXPECT_EQ(compiled->passes[0].colorTarget.load, LoadOp::DontCare);
    EXPECT_EQ(compiled->passes[0].colorTarget.store, StoreOp::Store);
}

TEST(RenderGraphTest, WritingColorAttachmentImportWithoutClearLoadsWithNoBarrier)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    graph.addPass(pass("p", 0, image));

    const auto compiled = graph.compile();
    ASSERT_TRUE(compiled.has_value());
    EXPECT_EQ(compiled->passes[0].colorTarget.load, LoadOp::Load);
    EXPECT_TRUE(compiled->passes[0].barriers.empty());
}

TEST(RenderGraphTest, ClearWinsOverDefinedContents)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    graph.addPass(pass("p", 0, image, std::array<float, 4>{0.25f, 0.5f, 0.75f, 1}));

    const auto compiled = graph.compile();
    ASSERT_TRUE(compiled.has_value());
    EXPECT_EQ(compiled->passes[0].colorTarget,
              (ColorTarget{LoadOp::Clear, StoreOp::Store, {0.25f, 0.5f, 0.75f, 1}}));
}

TEST(RenderGraphTest, ReadingAnUndefinedImportNoPassWritesIsInvalidArgumentNamingPass)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    const ResourceId other = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    PassDescription reader = pass("reader", 0, image);
    reader.reads = {other};
    graph.addPass(std::move(reader));
    expectInvalidNaming(graph, "reader", "no defined contents");
}

TEST(RenderGraphTest, DuplicatePassNameIsInvalidArgumentNamingPass)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    graph.addPass(pass("same", 0, image));
    graph.addPass(pass("same", 1, image));
    expectInvalidNaming(graph, "same");
}

TEST(RenderGraphTest, EmptyPassNameIsInvalidArgument)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    graph.addPass(pass("", 0, image));
    const auto result = graph.compile();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
}

TEST(RenderGraphTest, UnissuedResourceIsInvalidArgumentNamingPass)
{
    RenderGraph asTarget;
    asTarget.importTarget(SwapChainImage{}, ResourceState::Undefined);
    asTarget.addPass(pass("badTarget", 0, ResourceId{7}));
    expectInvalidNaming(asTarget, "badTarget");

    RenderGraph asRead;
    const ResourceId image = asRead.importTarget(SwapChainImage{}, ResourceState::Undefined);
    PassDescription reader = pass("badRead", 0, image);
    reader.reads = {ResourceId{7}};
    asRead.addPass(std::move(reader));
    expectInvalidNaming(asRead, "badRead");
}

TEST(RenderGraphTest, UnsetColorTargetIsInvalidArgumentNamingPass)
{
    RenderGraph graph;
    graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    PassDescription p;
    p.name = "unset";
    graph.addPass(std::move(p));
    expectInvalidNaming(graph, "unset", "color.target");
}

TEST(RenderGraphTest, ReadingOwnColorTargetIsInvalidArgumentNamingPass)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    PassDescription p = pass("self", 0, image);
    p.reads = {image};
    graph.addPass(std::move(p));
    expectInvalidNaming(graph, "self", "own colour target");
}

TEST(RenderGraphTest, ReadingTheSwapChainImageIsInvalidArgumentNamingPass)
{
    RenderGraph graph;
    const ResourceId target = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    graph.addPass(pass("writer", 0, image));
    PassDescription reader = pass("sampler", 1, target);
    reader.reads = {image};
    graph.addPass(std::move(reader));
    expectInvalidNaming(graph, "sampler", "swap chain");
}

TEST(RenderGraphTest, ReadingAnUndefinedImportAfterAPassWritesItFailsOnlyForTheSwapChain)
{
    RenderGraph graph;
    const ResourceId target = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    graph.addPass(pass("writer", 0, image));
    PassDescription reader = pass("sampler", 1, target);
    reader.reads = {image};
    graph.addPass(std::move(reader));
    expectInvalidNaming(graph, "sampler", "swap chain");
}

TEST(RenderGraphTest, BarrierAddedWhenEarlierPassWroteEvenInSameState)
{
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::ColorAttachment);
    graph.addPass(pass("one", 0, image));
    graph.addPass(pass("two", 1, image));

    const auto compiled = graph.compile();
    ASSERT_TRUE(compiled.has_value());
    EXPECT_TRUE(compiled->passes[0].barriers.empty());
    EXPECT_EQ(compiled->passes[1].barriers,
              (std::vector<Barrier>{
                  {SwapChainImage{}, ResourceState::ColorAttachment, ResourceState::ColorAttachment}}));
}

TEST(RenderGraphTest, CompilingTwiceGivesEqualResults)
{
    const RenderGraph graph = firstGraph();
    const auto first = graph.compile();
    const auto second = graph.compile();
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    ASSERT_EQ(first->passes.size(), second->passes.size());
    for (size_t i = 0; i < first->passes.size(); ++i)
    {
        EXPECT_EQ(first->passes[i].name, second->passes[i].name);
        EXPECT_EQ(first->passes[i].barriers, second->passes[i].barriers);
        EXPECT_EQ(first->passes[i].colorTarget, second->passes[i].colorTarget);
    }
}

TEST(RenderGraphTest, ClearRemovesPassesAndResources)
{
    RenderGraph graph = firstGraph();
    graph.clear();
    const auto empty = graph.compile();
    ASSERT_TRUE(empty.has_value());
    EXPECT_TRUE(empty->passes.empty());

    // The old ResourceId is no longer issued.
    graph.addPass(pass("p", 0, ResourceId{0}));
    expectInvalidNaming(graph, "p");

    graph.clear();
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    EXPECT_EQ(image, ResourceId{0});
}

TEST(RenderGraphTest, ExecuteRecordsBarriersRenderingAndCallbacksInOrder)
{
    FakeDevice device;
    ASSERT_TRUE(device.attachWindow(NativeWindow{Win32Handles{}}, {640, 480}).has_value());
    auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());

    std::vector<std::string> log;
    const RenderGraph graph = firstGraph(&log);
    ASSERT_TRUE(graph.execute(**frame).has_value());
    ASSERT_TRUE(device.endFrame().has_value());

    const std::vector<RecordedCommand> expected{
        Barrier{SwapChainImage{}, ResourceState::Undefined, ResourceState::ColorAttachment},
        BeginRenderingCommand{ColorTarget{LoadOp::Clear, StoreOp::Store, kBlack}},
        DrawCommand{2, 1, 0, 0},
        EndRenderingCommand{},
        Barrier{SwapChainImage{}, ResourceState::ColorAttachment, ResourceState::ColorAttachment},
        BeginRenderingCommand{ColorTarget{LoadOp::Load, StoreOp::Store, {0, 0, 0, 1}}},
        DrawCommand{1, 1, 0, 0},
        EndRenderingCommand{},
    };
    ASSERT_EQ(device.frames().size(), 1u);
    EXPECT_EQ(device.frames()[0], expected);
    EXPECT_EQ(log, (std::vector<std::string>{"b", "a"}));
}

TEST(RenderGraphTest, ExecuteRecordsNothingWhenCompileFails)
{
    FakeDevice device;
    ASSERT_TRUE(device.attachWindow(NativeWindow{Win32Handles{}}, {640, 480}).has_value());
    auto frame = device.beginFrame();
    ASSERT_TRUE(frame.has_value());

    bool called = false;
    RenderGraph graph;
    const ResourceId image = graph.importTarget(SwapChainImage{}, ResourceState::Undefined);
    graph.addPass(pass("good", 0, image));
    PassDescription bad = pass("good", 1, image);
    bad.record = [&called](CommandList &) { called = true; };
    graph.addPass(std::move(bad));

    const auto result = graph.execute(**frame);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
    ASSERT_TRUE(device.endFrame().has_value());
    EXPECT_FALSE(called);
    EXPECT_TRUE(device.frames()[0].empty());
}
