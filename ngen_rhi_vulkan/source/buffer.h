// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/rhi/descriptions.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace ngen::rhi::vulkan
{
    // A buffer handle's resource: a VkBuffer in host-visible, coherent memory from VMA, mapped for its whole
    // life. Move-only; destroying it frees the buffer and its memory.
    class Buffer
    {
    public:
        // The description has already been validated.
        [[nodiscard]] static std::expected<Buffer, ngen::Error> create(VmaAllocator allocator,
                                                                       const BufferDescription &description);

        Buffer(Buffer &&other) noexcept;
        Buffer &operator=(Buffer &&other) noexcept;
        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;
        ~Buffer();

        // Copies into the mapped memory; InvalidArgument when the range runs past the end.
        [[nodiscard]] std::expected<void, ngen::Error> write(uint64_t offset, std::span<const std::byte> data);

        [[nodiscard]] VkBuffer handle() const
        {
            return buffer_;
        }

        [[nodiscard]] uint64_t size() const
        {
            return size_;
        }

        [[nodiscard]] BufferUsage usage() const
        {
            return usage_;
        }

    private:
        Buffer(VmaAllocator allocator, uint64_t size, BufferUsage usage);
        void release();

        VmaAllocator allocator_ = VK_NULL_HANDLE;
        VkBuffer buffer_ = VK_NULL_HANDLE;
        VmaAllocation allocation_ = VK_NULL_HANDLE;
        std::byte *mapped_ = nullptr;
        uint64_t size_ = 0;
        BufferUsage usage_ = BufferUsage::Vertex;
    };
} // namespace ngen::rhi::vulkan
