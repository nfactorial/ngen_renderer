// Spec: rhi. Behaviour changes go through it.
#include <ngen/rhi/descriptions.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

using namespace ngen::rhi;

namespace
{
    void expectInvalidNaming(const std::expected<void, ngen::Error> &result, const std::string &field)
    {
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, ngen::ErrorCode::InvalidArgument);
        EXPECT_NE(result.error().message.find(field), std::string::npos) << result.error().message;
    }

    GraphicsPipelineDescription validPipeline()
    {
        GraphicsPipelineDescription description;
        description.vertex = {ShaderHandle{0, 1}, "main"};
        description.fragment = {ShaderHandle{1, 1}, "main"};
        return description;
    }
} // namespace

TEST(BufferDescription, ZeroSizeIsInvalid)
{
    expectInvalidNaming(validate(BufferDescription{0, BufferUsage::Vertex}), "size");
}

TEST(BufferDescription, EmptyUsageIsInvalid)
{
    expectInvalidNaming(validate(BufferDescription{16, static_cast<BufferUsage>(0)}), "usage");
}

TEST(BufferDescription, UsageFlagsCombine)
{
    EXPECT_TRUE(validate(BufferDescription{16, BufferUsage::Vertex}).has_value());
    EXPECT_TRUE(validate(BufferDescription{16, BufferUsage::Uniform}).has_value());
    EXPECT_TRUE(validate(BufferDescription{16, BufferUsage::Vertex | BufferUsage::Uniform}).has_value());
}

TEST(ShaderDescription, EmptyBytecodeIsInvalid)
{
    expectInvalidNaming(validate(ShaderDescription{}), "bytecode");
}

TEST(ShaderDescription, NonEmptyBytecodeIsValid)
{
    const std::array<std::byte, 4> code{};
    EXPECT_TRUE(validate(ShaderDescription{code}).has_value());
}

TEST(GraphicsPipelineDescription, DuplicateBindingNumberIsInvalid)
{
    GraphicsPipelineDescription description = validPipeline();
    description.bindings = {{0, 12}, {0, 24}};
    expectInvalidNaming(validate(description), "bindings[1].binding");
}

TEST(GraphicsPipelineDescription, EmptyEntryPointsAreInvalid)
{
    GraphicsPipelineDescription description = validPipeline();
    description.vertex.entryPoint.clear();
    expectInvalidNaming(validate(description), "vertex.entryPoint");

    description = validPipeline();
    description.fragment.entryPoint.clear();
    expectInvalidNaming(validate(description), "fragment.entryPoint");

    EXPECT_TRUE(validate(validPipeline()).has_value());
}

TEST(GraphicsPipelineDescription, AttributeNamingMissingBindingIsInvalid)
{
    GraphicsPipelineDescription description = validPipeline();
    description.bindings = {{0, 12}};
    description.attributes = {{0, 1, Format::R32G32B32Float, 0}};
    expectInvalidNaming(validate(description), "attributes[0].binding");
}

TEST(GraphicsPipelineDescription, DuplicateLocationIsInvalid)
{
    GraphicsPipelineDescription description = validPipeline();
    description.bindings = {{0, 24}};
    description.attributes = {{0, 0, Format::R32G32Float, 0}, {0, 0, Format::R32G32Float, 8}};
    expectInvalidNaming(validate(description), "attributes[1].location");
}

TEST(GraphicsPipelineDescription, ConsistentAttributesAreValid)
{
    GraphicsPipelineDescription description = validPipeline();
    description.bindings = {{0, 24}};
    description.attributes = {{0, 0, Format::R32G32Float, 0}, {1, 0, Format::R32G32B32Float, 8}};
    EXPECT_TRUE(validate(description).has_value());
}

TEST(GraphicsPipelineDescription, DefaultsMatchTheSpec)
{
    const GraphicsPipelineDescription description;
    EXPECT_EQ(description.topology, Topology::TriangleList);
    EXPECT_EQ(description.cull, CullMode::Back);
    EXPECT_EQ(description.frontFace, FrontFace::Clockwise);
    EXPECT_EQ(description.polygonMode, PolygonMode::Fill);
}

TEST(ColorTarget, DefaultsMatchTheSpec)
{
    const ColorTarget target;
    EXPECT_EQ(target.load, LoadOp::Clear);
    EXPECT_EQ(target.store, StoreOp::Store);
    EXPECT_EQ(target.clearColor, (std::array<float, 4>{0, 0, 0, 1}));
}

TEST(Barrier, FromTheTrackedStateToAnyOtherUseIsValid)
{
    EXPECT_TRUE(validate(Barrier{SwapChainImage{}, ResourceState::Undefined, ResourceState::ColorAttachment},
                         ResourceState::Undefined)
                    .has_value());
    EXPECT_TRUE(validate(Barrier{SwapChainImage{}, ResourceState::ColorAttachment, ResourceState::ColorAttachment},
                         ResourceState::ColorAttachment)
                    .has_value());
    EXPECT_TRUE(validate(Barrier{SwapChainImage{}, ResourceState::Present, ResourceState::Present},
                         ResourceState::Present)
                    .has_value());
}

TEST(Barrier, BeforeThatIsNotTheTrackedStateIsInvalidNamingBothStates)
{
    const auto result = validate(Barrier{SwapChainImage{}, ResourceState::Present, ResourceState::ColorAttachment},
                                 ResourceState::ColorAttachment);
    expectInvalidNaming(result, "Present");
    expectInvalidNaming(result, "ColorAttachment");
}

TEST(Barrier, AfterOfUndefinedIsInvalid)
{
    expectInvalidNaming(validate(Barrier{SwapChainImage{}, ResourceState::ColorAttachment, ResourceState::Undefined},
                                 ResourceState::ColorAttachment),
                        "Undefined");
}

TEST(Barrier, TheSwapChainImageIsNeverShaderRead)
{
    expectInvalidNaming(validate(Barrier{SwapChainImage{}, ResourceState::Undefined, ResourceState::ShaderRead},
                                 ResourceState::Undefined),
                        "ShaderRead");
    expectInvalidNaming(validate(Barrier{SwapChainImage{}, ResourceState::ShaderRead, ResourceState::Present},
                                 ResourceState::ShaderRead),
                        "ShaderRead");
}
