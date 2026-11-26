// Spec: rhi-vulkan. Behaviour changes go through it.
// Real Vulkan with validation on and WindowSupport::None: the device never presents.
#include "test_support.h"
#include "vulkan_context.h"

#include <ngen/rhi/vulkan/vulkan_device.h>

#include <gtest/gtest.h>

#include <string>
#include <utility>

using namespace ngen::rhi;
using namespace ngen::rhi::vulkan;
using ngen::ErrorCode;
using test_support::MessageLog;

namespace
{
    std::unique_ptr<Device> makeDevice(const MessageLog &log)
    {
        auto device = VulkanDevice::create(DeviceDescription{"ngen_rhi_vulkan_tests", true, log.sink()},
                                           WindowSupport::None);
        EXPECT_TRUE(device.has_value()) << (device.has_value() ? "" : device.error().message);
        return device.has_value() ? std::move(*device) : nullptr;
    }
} // namespace

// Acceptance: creating the device, a 1 KB vertex buffer, writing 36 bytes, a shader from the test's Slang module
// and a pipeline using both entries, then destroying them and the device, sends no Warning or Error.
TEST(VulkanDeviceSmoke, CreatesAndDestroysResourcesWithNoValidationMessages)
{
    const MessageLog log;
    {
        auto device = makeDevice(log);
        ASSERT_NE(device, nullptr);

        const auto buffer = device->createBuffer({1024, BufferUsage::Vertex});
        ASSERT_TRUE(buffer.has_value()) << buffer.error().message;
        const std::vector<std::byte> vertices = test_support::triangleVertices();
        ASSERT_EQ(vertices.size(), 36u);
        const auto written = device->writeBuffer(*buffer, 0, vertices);
        ASSERT_TRUE(written.has_value()) << written.error().message;

        const std::vector<std::byte> code = test_support::loadTestShader();
        ASSERT_FALSE(code.empty()) << "the compiled test shader is missing: " << NGEN_TEST_SHADER_PATH;
        const auto shader = device->createShader({code});
        ASSERT_TRUE(shader.has_value()) << shader.error().message;

        const auto pipeline = device->createPipeline(test_support::testPipelineDescription(*shader));
        ASSERT_TRUE(pipeline.has_value()) << pipeline.error().message;

        device->destroy(*pipeline);
        device->destroy(*shader);
        device->destroy(*buffer);
    }
    EXPECT_EQ(log.problems(), "");
}

// Acceptance: writeBuffer of 2 KB into the 1 KB buffer returns InvalidArgument.
TEST(VulkanDeviceSmoke, WriteBufferPastTheEndIsInvalidArgument)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const auto buffer = device->createBuffer({1024, BufferUsage::Vertex});
    ASSERT_TRUE(buffer.has_value());
    const std::vector<std::byte> data(2048);
    const auto written = device->writeBuffer(*buffer, 0, data);
    ASSERT_FALSE(written.has_value());
    EXPECT_EQ(written.error().code, ErrorCode::InvalidArgument);

    // An offset that leaves too little room is past the end too; exactly filling the buffer is not.
    const auto offsetWrite = device->writeBuffer(*buffer, 1000, std::span(data).first(100));
    ASSERT_FALSE(offsetWrite.has_value());
    EXPECT_EQ(offsetWrite.error().code, ErrorCode::InvalidArgument);
    EXPECT_TRUE(device->writeBuffer(*buffer, 1000, std::span(data).first(24)).has_value());
}

// Acceptance: with WindowSupport::None, attachWindow returns Unsupported.
TEST(VulkanDeviceSmoke, AttachWindowIsUnsupported)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const auto attached = device->attachWindow(ngen::platform::Win32Handles{nullptr, nullptr}, {640, 480});
    ASSERT_FALSE(attached.has_value());
    EXPECT_EQ(attached.error().code, ErrorCode::Unsupported);
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, MissingValidationLayerIsUnsupportedNamingTheLayer)
{
    // The context is asked for a validation layer no machine has installed, so the check does not depend on
    // which layers this machine has, or on loader environment variables.
    constexpr const char *missingLayer = "VK_LAYER_NGEN_not_installed";
    const MessageLog log;
    const auto context = VulkanContext::create("ngen_rhi_vulkan_tests", true, false, log.sink(), missingLayer);
    ASSERT_FALSE(context.has_value());
    EXPECT_EQ(context.error().code, ErrorCode::Unsupported);
    EXPECT_NE(context.error().message.find(missingLayer), std::string::npos) << context.error().message;
    EXPECT_NE(context.error().message.find("missing"), std::string::npos) << context.error().message;
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, ValidationRequestsTheKhronosLayer)
{
    // The layer VulkanDevice::create enables when validation is requested.
    EXPECT_STREQ(kValidationLayer, "VK_LAYER_KHRONOS_validation");
}

TEST(VulkanDeviceSmoke, WithoutValidationTheDeviceIsCreated)
{
    const MessageLog log;
    auto device =
        VulkanDevice::create(DeviceDescription{"ngen_rhi_vulkan_tests", false, log.sink()}, WindowSupport::None);
    ASSERT_TRUE(device.has_value()) << device.error().message;
}

TEST(VulkanDeviceSmoke, WithNoWindowBeginFrameReturnsNullptrAndThereIsNoSwapChain)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const auto frame = device->beginFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, nullptr);
    EXPECT_EQ(device->swapChainSize(), (ngen::platform::PixelSize{0, 0}));

    // Window events with no window attached are ignored, and onResumed is InvalidArgument.
    device->onResized({800, 600});
    device->onSuspended();
    const auto resumed = device->onResumed(ngen::platform::Win32Handles{nullptr, nullptr}, {800, 600});
    ASSERT_FALSE(resumed.has_value());
    EXPECT_EQ(resumed.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(device->swapChainSize(), (ngen::platform::PixelSize{0, 0}));
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, EndFrameWithoutAFrameIsInvalidArgumentAndReportsAnError)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const auto ended = device->endFrame();
    ASSERT_FALSE(ended.has_value());
    EXPECT_EQ(ended.error().code, ErrorCode::InvalidArgument);
    ASSERT_EQ(log.messages().size(), 1u);
    EXPECT_EQ(log.messages()[0].severity, Severity::Error);
}

TEST(VulkanDeviceSmoke, InvalidDescriptionsAreInvalidArgument)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const auto noSize = device->createBuffer({0, BufferUsage::Vertex});
    ASSERT_FALSE(noSize.has_value());
    EXPECT_EQ(noSize.error().code, ErrorCode::InvalidArgument);

    const auto noCode = device->createShader({});
    ASSERT_FALSE(noCode.has_value());
    EXPECT_EQ(noCode.error().code, ErrorCode::InvalidArgument);

    const std::vector<std::byte> code = test_support::loadTestShader();
    const auto shader = device->createShader({code});
    ASSERT_TRUE(shader.has_value());
    GraphicsPipelineDescription badBinding = test_support::testPipelineDescription(*shader);
    badBinding.attributes[0].binding = 1;
    const auto pipeline = device->createPipeline(badBinding);
    ASSERT_FALSE(pipeline.has_value());
    EXPECT_EQ(pipeline.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, BytecodeThatIsNotSpirvIsInvalidArgument)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const std::vector<std::byte> oddSize(6, std::byte{0});
    const auto odd = device->createShader({oddSize});
    ASSERT_FALSE(odd.has_value());
    EXPECT_EQ(odd.error().code, ErrorCode::InvalidArgument);

    const std::vector<std::byte> noMagic(8, std::byte{0});
    const auto wrong = device->createShader({noMagic});
    ASSERT_FALSE(wrong.has_value());
    EXPECT_EQ(wrong.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, PipelineNamingADestroyedShaderIsInvalidArgument)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const std::vector<std::byte> code = test_support::loadTestShader();
    const auto shader = device->createShader({code});
    ASSERT_TRUE(shader.has_value());
    device->destroy(*shader);

    const auto pipeline = device->createPipeline(test_support::testPipelineDescription(*shader));
    ASSERT_FALSE(pipeline.has_value());
    EXPECT_EQ(pipeline.error().code, ErrorCode::InvalidArgument);

    const auto null = device->createPipeline(test_support::testPipelineDescription(ShaderHandle{}));
    ASSERT_FALSE(null.has_value());
    EXPECT_EQ(null.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, StaleAndNullHandlesAreIgnoredOrRejected)
{
    const MessageLog log;
    auto device = makeDevice(log);
    ASSERT_NE(device, nullptr);

    const auto buffer = device->createBuffer({64, BufferUsage::Vertex | BufferUsage::Uniform});
    ASSERT_TRUE(buffer.has_value());
    device->destroy(*buffer);
    device->destroy(*buffer);
    device->destroy(BufferHandle{});
    device->destroy(ShaderHandle{});
    device->destroy(PipelineHandle{});

    const std::vector<std::byte> data(4);
    const auto written = device->writeBuffer(*buffer, 0, data);
    ASSERT_FALSE(written.has_value());
    EXPECT_EQ(written.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(log.problems(), "");
}

TEST(VulkanDeviceSmoke, EveryPipelineStateCreatesAValidPipeline)
{
    const MessageLog log;
    {
        auto device = makeDevice(log);
        ASSERT_NE(device, nullptr);
        const std::vector<std::byte> code = test_support::loadTestShader();
        const auto shader = device->createShader({code});
        ASSERT_TRUE(shader.has_value());

        GraphicsPipelineDescription description = test_support::testPipelineDescription(*shader);
        description.bindings[0].perInstance = true;
        for (const Topology topology : {Topology::TriangleList, Topology::TriangleStrip, Topology::LineList})
        {
            for (const CullMode cull : {CullMode::None, CullMode::Front, CullMode::Back})
            {
                description.topology = topology;
                description.cull = cull;
                description.frontFace = cull == CullMode::Back ? FrontFace::Clockwise : FrontFace::CounterClockwise;
                const auto pipeline = device->createPipeline(description);
                ASSERT_TRUE(pipeline.has_value()) << pipeline.error().message;
                device->destroy(*pipeline);
            }
        }

        // Line mode needs a device feature: it works where the device has it, and is Unsupported elsewhere.
        description.polygonMode = PolygonMode::Line;
        const auto lines = device->createPipeline(description);
        if (!lines.has_value())
        {
            EXPECT_EQ(lines.error().code, ErrorCode::Unsupported) << lines.error().message;
        }
    }
    EXPECT_EQ(log.problems(), "");
}
