// Spec: rhi-vulkan. Behaviour changes go through it.
#include "device_selection.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace ngen::rhi::vulkan
{
    std::expected<DeviceChoice, std::string> choosePhysicalDevice(
        const std::span<const PhysicalDeviceCandidate> candidates, const bool windowExpected)
    {
        if (candidates.empty())
        {
            return std::unexpected(std::string("no Vulkan physical devices were found"));
        }

        std::optional<DeviceChoice> chosen;
        bool chosenIsDiscrete = false;
        std::string rejections;

        for (size_t index = 0; index < candidates.size(); ++index)
        {
            const PhysicalDeviceCandidate &candidate = candidates[index];

            std::vector<std::string> reasons;
            if (!candidate.graphicsFamily.has_value())
            {
                reasons.emplace_back("has no graphics queue family");
            }
            if (candidate.apiVersion < VK_API_VERSION_1_1)
            {
                reasons.emplace_back("does not support Vulkan 1.1");
            }
            if (windowExpected && !candidate.hasSwapchain)
            {
                reasons.emplace_back("does not offer VK_KHR_swapchain");
            }
            if (windowExpected && candidate.graphicsFamily.has_value() && !candidate.presentFamily.has_value())
            {
                reasons.emplace_back("has no graphics queue family that can present");
            }

            if (!reasons.empty())
            {
                rejections += rejections.empty() ? "" : "; ";
                rejections += candidate.name + " ";
                for (size_t i = 0; i < reasons.size(); ++i)
                {
                    rejections += (i == 0 ? "" : ", ") + reasons[i];
                }
                continue;
            }

            const uint32_t family = windowExpected ? *candidate.presentFamily : *candidate.graphicsFamily;
            if (!chosen.has_value() || (candidate.discrete && !chosenIsDiscrete))
            {
                chosen = DeviceChoice{index, family};
                chosenIsDiscrete = candidate.discrete;
            }
        }

        if (!chosen.has_value())
        {
            return std::unexpected("no suitable Vulkan device: " + rejections);
        }
        return *chosen;
    }
} // namespace ngen::rhi::vulkan
