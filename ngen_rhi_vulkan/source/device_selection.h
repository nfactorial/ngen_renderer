// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>

namespace ngen::rhi::vulkan
{
    // What device selection needs to know about one physical device.
    struct PhysicalDeviceCandidate
    {
        std::string name;
        bool discrete = false;
        uint32_t apiVersion = 0;
        std::optional<uint32_t> graphicsFamily; // the first family with a graphics queue
        std::optional<uint32_t> presentFamily;  // the first graphics family the platform reports can present
        bool hasSwapchain = false;              // offers VK_KHR_swapchain
    };

    struct DeviceChoice
    {
        size_t index;         // into the candidates
        uint32_t queueFamily; // the one family used for graphics and presentation
    };

    // A suitable device has a graphics queue family and Vulkan 1.1 and, when a window may be attached, the
    // swap chain extension and a graphics family that can present. A discrete GPU is preferred over any other
    // suitable device; otherwise the first suitable one is chosen. With none, the error says why each device
    // was rejected.
    [[nodiscard]] std::expected<DeviceChoice, std::string> choosePhysicalDevice(
        std::span<const PhysicalDeviceCandidate> candidates, bool windowExpected);
} // namespace ngen::rhi::vulkan
