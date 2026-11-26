// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/rhi/device.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>

namespace ngen::rhi::vulkan
{
    // The layer validation enables.
    inline constexpr const char *kValidationLayer = "VK_LAYER_KHRONOS_validation";

    // The instance, debug messenger, physical and logical device, queue and memory allocator, and the message
    // sink everything is reported through. The destructor destroys them in reverse order of creation.
    class VulkanContext
    {
    public:
        // windowExpected enables the surface and swap chain extensions and requires a queue family that can
        // present. An empty sink is replaced by makeDefaultMessageSink(). validationLayer is the layer
        // enableValidation turns on; only tests name another (one that is not installed).
        [[nodiscard]] static std::expected<std::unique_ptr<VulkanContext>, ngen::Error> create(
            const std::string &applicationName, bool enableValidation, bool windowExpected, MessageSink sink,
            const char *validationLayer = kValidationLayer);

        ~VulkanContext();
        VulkanContext(const VulkanContext &) = delete;
        VulkanContext &operator=(const VulkanContext &) = delete;

        void report(Severity severity, std::string_view message) const;

        [[nodiscard]] VkInstance instance() const
        {
            return instance_;
        }

        [[nodiscard]] VkPhysicalDevice physicalDevice() const
        {
            return physicalDevice_;
        }

        [[nodiscard]] VkDevice device() const
        {
            return device_;
        }

        [[nodiscard]] VkQueue queue() const
        {
            return queue_;
        }

        [[nodiscard]] uint32_t queueFamily() const
        {
            return queueFamily_;
        }

        [[nodiscard]] VmaAllocator allocator() const
        {
            return allocator_;
        }

        [[nodiscard]] bool windowExpected() const
        {
            return windowExpected_;
        }

        [[nodiscard]] const VkPhysicalDeviceLimits &limits() const
        {
            return limits_;
        }

        // Whether PolygonMode::Line can be used.
        [[nodiscard]] bool fillModeNonSolid() const
        {
            return fillModeNonSolid_;
        }

    private:
        VulkanContext(MessageSink sink, bool windowExpected);

        [[nodiscard]] std::expected<void, ngen::Error> createInstance(const std::string &applicationName,
                                                                      bool enableValidation,
                                                                      const char *validationLayer);
        [[nodiscard]] std::expected<void, ngen::Error> selectPhysicalDevice();
        [[nodiscard]] std::expected<void, ngen::Error> createDevice();
        [[nodiscard]] std::expected<void, ngen::Error> createAllocator();

        MessageSink sink_;
        bool windowExpected_;
        VkInstance instance_ = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
        VkPhysicalDeviceLimits limits_{};
        bool hasPortabilitySubset_ = false;
        bool fillModeNonSolid_ = false;
        uint32_t queueFamily_ = 0;
        VkDevice device_ = VK_NULL_HANDLE;
        VkQueue queue_ = VK_NULL_HANDLE;
        VmaAllocator allocator_ = VK_NULL_HANDLE;
    };
} // namespace ngen::rhi::vulkan
