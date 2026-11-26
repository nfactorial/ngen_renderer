// Spec: rhi-vulkan. Behaviour changes go through it.
// The backend's selection rules and barrier mapping, checked without a device.
#include "command_list.h"
#include "device_selection.h"
#include "swap_chain.h"

#include <gtest/gtest.h>

#include <limits>
#include <vector>

using namespace ngen::rhi::vulkan;

namespace
{
    PhysicalDeviceCandidate suitable(const std::string &name, const bool discrete, const uint32_t family = 0)
    {
        PhysicalDeviceCandidate candidate;
        candidate.name = name;
        candidate.discrete = discrete;
        candidate.apiVersion = VK_API_VERSION_1_1;
        candidate.graphicsFamily = family;
        candidate.presentFamily = family;
        candidate.hasSwapchain = true;
        return candidate;
    }
} // namespace

TEST(DeviceSelection, ADiscreteGpuIsPreferredOverAnEarlierIntegratedOne)
{
    const std::vector<PhysicalDeviceCandidate> candidates = {suitable("integrated", false),
                                                             suitable("discrete", true)};
    const auto choice = choosePhysicalDevice(candidates, true);
    ASSERT_TRUE(choice.has_value()) << choice.error();
    EXPECT_EQ(choice->index, 1u);
}

TEST(DeviceSelection, WithNoDiscreteGpuTheFirstSuitableDeviceIsChosen)
{
    PhysicalDeviceCandidate old = suitable("old", false);
    old.apiVersion = VK_API_VERSION_1_0;
    const std::vector<PhysicalDeviceCandidate> candidates = {old, suitable("first", false),
                                                             suitable("second", false)};
    const auto choice = choosePhysicalDevice(candidates, true);
    ASSERT_TRUE(choice.has_value()) << choice.error();
    EXPECT_EQ(choice->index, 1u);
}

TEST(DeviceSelection, AnUnsuitableDiscreteGpuLosesToASuitableIntegratedOne)
{
    PhysicalDeviceCandidate discrete = suitable("discrete", true);
    discrete.hasSwapchain = false;
    const std::vector<PhysicalDeviceCandidate> candidates = {discrete, suitable("integrated", false)};
    const auto choice = choosePhysicalDevice(candidates, true);
    ASSERT_TRUE(choice.has_value()) << choice.error();
    EXPECT_EQ(choice->index, 1u);
}

TEST(DeviceSelection, WithAWindowTheQueueFamilyIsTheFirstGraphicsFamilyThatCanPresent)
{
    PhysicalDeviceCandidate candidate = suitable("gpu", true);
    candidate.graphicsFamily = 0;
    candidate.presentFamily = 2;
    const std::vector<PhysicalDeviceCandidate> candidates = {candidate};

    const auto windowed = choosePhysicalDevice(candidates, true);
    ASSERT_TRUE(windowed.has_value()) << windowed.error();
    EXPECT_EQ(windowed->queueFamily, 2u);

    const auto offscreen = choosePhysicalDevice(candidates, false);
    ASSERT_TRUE(offscreen.has_value()) << offscreen.error();
    EXPECT_EQ(offscreen->queueFamily, 0u);
}

TEST(DeviceSelection, WithoutAWindowNoSwapChainOrPresentationIsNeeded)
{
    PhysicalDeviceCandidate headless = suitable("headless", false);
    headless.hasSwapchain = false;
    headless.presentFamily.reset();
    const std::vector<PhysicalDeviceCandidate> candidates = {headless};

    EXPECT_TRUE(choosePhysicalDevice(candidates, false).has_value());
    EXPECT_FALSE(choosePhysicalDevice(candidates, true).has_value());
}

TEST(DeviceSelection, WithNoSuitableDeviceTheErrorSaysWhyEachWasRejected)
{
    PhysicalDeviceCandidate noGraphics = suitable("compute-only", true);
    noGraphics.graphicsFamily.reset();
    noGraphics.presentFamily.reset();
    PhysicalDeviceCandidate old = suitable("vulkan-1.0", false);
    old.apiVersion = VK_API_VERSION_1_0;
    PhysicalDeviceCandidate cannotPresent = suitable("no-present", false);
    cannotPresent.presentFamily.reset();
    cannotPresent.hasSwapchain = false;
    const std::vector<PhysicalDeviceCandidate> candidates = {noGraphics, old, cannotPresent};

    const auto choice = choosePhysicalDevice(candidates, true);
    ASSERT_FALSE(choice.has_value());
    const std::string &message = choice.error();
    EXPECT_NE(message.find("compute-only has no graphics queue family"), std::string::npos) << message;
    EXPECT_NE(message.find("vulkan-1.0 does not support Vulkan 1.1"), std::string::npos) << message;
    EXPECT_NE(message.find("no-present does not offer VK_KHR_swapchain"), std::string::npos) << message;
    EXPECT_NE(message.find("has no graphics queue family that can present"), std::string::npos) << message;
}

TEST(DeviceSelection, NoDevicesAtAllIsAnError)
{
    const auto choice = choosePhysicalDevice({}, false);
    ASSERT_FALSE(choice.has_value());
    EXPECT_NE(choice.error().find("no Vulkan physical devices"), std::string::npos);
}

TEST(SwapChainChoice, PrefersBgra8UnormWithSrgbNonLinearElseTheFirstFormat)
{
    const std::vector<VkSurfaceFormatKHR> withPreferred = {
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    const VkSurfaceFormatKHR chosen = chooseSurfaceFormat(withPreferred);
    EXPECT_EQ(chosen.format, VK_FORMAT_B8G8R8A8_UNORM);
    EXPECT_EQ(chosen.colorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

    const std::vector<VkSurfaceFormatKHR> without = {
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT}};
    EXPECT_EQ(chooseSurfaceFormat(without).format, VK_FORMAT_R8G8B8A8_UNORM);
}

TEST(SwapChainChoice, PrefersFifo)
{
    const std::vector<VkPresentModeKHR> modes = {VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
                                                 VK_PRESENT_MODE_FIFO_KHR};
    EXPECT_EQ(choosePresentMode(modes), VK_PRESENT_MODE_FIFO_KHR);
}

TEST(SwapChainChoice, TheExtentIsTheSurfacesUnlessTheSurfaceLetsTheSwapChainDecide)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.currentExtent = {1024, 768};
    capabilities.minImageExtent = {1, 1};
    capabilities.maxImageExtent = {4096, 4096};
    EXPECT_EQ(chooseExtent(capabilities, {800, 600}).width, 1024u);
    EXPECT_EQ(chooseExtent(capabilities, {800, 600}).height, 768u);

    capabilities.currentExtent = {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
    EXPECT_EQ(chooseExtent(capabilities, {800, 600}).width, 800u);
    EXPECT_EQ(chooseExtent(capabilities, {800, 600}).height, 600u);
    EXPECT_EQ(chooseExtent(capabilities, {8000, 600}).width, 4096u);
}

TEST(SwapChainChoice, TheImageCountIsOneAboveTheMinimumWithinTheMaximum)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.minImageCount = 2;
    capabilities.maxImageCount = 0; // no maximum
    EXPECT_EQ(chooseImageCount(capabilities), 3u);
    capabilities.maxImageCount = 2;
    EXPECT_EQ(chooseImageCount(capabilities), 2u);
}

TEST(SwapChainChoice, CompositeAlphaIsOpaqueWhenSupportedElseTheFirstSupported)
{
    EXPECT_EQ(chooseCompositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR),
              VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    EXPECT_EQ(chooseCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR), VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
}

TEST(BarrierMapping, EachStateMapsToItsLayoutStageAndAccess)
{
    using ngen::rhi::ResourceState;
    const auto expectUse = [](const ImageStateUse &use, const VkImageLayout layout, const VkPipelineStageFlags stage,
                              const VkAccessFlags access, const char *const state) {
        EXPECT_EQ(use.layout, layout) << state;
        EXPECT_EQ(use.stage, stage) << state;
        EXPECT_EQ(use.access, access) << state;
    };
    for (const bool presents : {true, false})
    {
        expectUse(imageStateUse(ResourceState::Undefined, presents), VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, "Undefined");
        expectUse(imageStateUse(ResourceState::ColorAttachment, presents), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, "ColorAttachment");
        expectUse(imageStateUse(ResourceState::ShaderRead, presents), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, "ShaderRead");
    }
    expectUse(imageStateUse(ResourceState::Present, true), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
              VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, "Present");
    // PRESENT_SRC_KHR needs VK_KHR_swapchain, which a WindowSupport::None device does not enable.
    expectUse(imageStateUse(ResourceState::Present, false), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
              VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, "Present without a window");
}

TEST(RenderPassChoice, ALoadPassWhileTheImageIsUndefinedUsesTheDontCarePass)
{
    using ngen::rhi::LoadOp;
    using ngen::rhi::ResourceState;
    EXPECT_EQ(passLoad(LoadOp::Load, ResourceState::Undefined), LoadOp::DontCare);
    EXPECT_EQ(passLoad(LoadOp::Load, ResourceState::ColorAttachment), LoadOp::Load);
    EXPECT_EQ(passLoad(LoadOp::Load, ResourceState::Present), LoadOp::Load);
    for (const ResourceState state : {ResourceState::Undefined, ResourceState::ColorAttachment, ResourceState::Present})
    {
        EXPECT_EQ(passLoad(LoadOp::Clear, state), LoadOp::Clear);
        EXPECT_EQ(passLoad(LoadOp::DontCare, state), LoadOp::DontCare);
    }
}
