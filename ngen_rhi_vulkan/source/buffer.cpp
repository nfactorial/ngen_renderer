// Spec: rhi-vulkan. Behaviour changes go through it.
#include "buffer.h"

#include "vulkan_result.h"

#include <cstring>
#include <utility>

namespace ngen::rhi::vulkan
{
    namespace
    {
        bool hasUsage(const BufferUsage usage, const BufferUsage flag)
        {
            return (usage & flag) != static_cast<BufferUsage>(0);
        }
    } // namespace

    std::expected<Buffer, ngen::Error> Buffer::create(const VmaAllocator allocator,
                                                      const BufferDescription &description)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = description.size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (hasUsage(description.usage, BufferUsage::Vertex))
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (hasUsage(description.usage, BufferUsage::Uniform))
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        Buffer buffer(allocator, description.size, description.usage);
        VmaAllocationInfo allocated{};
        if (const VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &buffer.buffer_,
                                                    &buffer.allocation_, &allocated);
            result != VK_SUCCESS)
        {
            buffer.buffer_ = VK_NULL_HANDLE;
            buffer.allocation_ = VK_NULL_HANDLE;
            return std::unexpected(vulkanError("vmaCreateBuffer", result));
        }
        buffer.mapped_ = static_cast<std::byte *>(allocated.pMappedData);
        return buffer;
    }

    Buffer::Buffer(const VmaAllocator allocator, const uint64_t size, const BufferUsage usage)
        : allocator_(allocator), size_(size), usage_(usage)
    {
    }

    Buffer::Buffer(Buffer &&other) noexcept
        : allocator_(std::exchange(other.allocator_, VK_NULL_HANDLE)),
          buffer_(std::exchange(other.buffer_, VK_NULL_HANDLE)),
          allocation_(std::exchange(other.allocation_, VK_NULL_HANDLE)), mapped_(std::exchange(other.mapped_, nullptr)),
          size_(other.size_), usage_(other.usage_)
    {
    }

    Buffer &Buffer::operator=(Buffer &&other) noexcept
    {
        if (this != &other)
        {
            release();
            allocator_ = std::exchange(other.allocator_, VK_NULL_HANDLE);
            buffer_ = std::exchange(other.buffer_, VK_NULL_HANDLE);
            allocation_ = std::exchange(other.allocation_, VK_NULL_HANDLE);
            mapped_ = std::exchange(other.mapped_, nullptr);
            size_ = other.size_;
            usage_ = other.usage_;
        }
        return *this;
    }

    Buffer::~Buffer()
    {
        release();
    }

    void Buffer::release()
    {
        if (buffer_ != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator_, buffer_, allocation_);
            buffer_ = VK_NULL_HANDLE;
            allocation_ = VK_NULL_HANDLE;
            mapped_ = nullptr;
        }
    }

    std::expected<void, ngen::Error> Buffer::write(const uint64_t offset, const std::span<const std::byte> data)
    {
        if (offset > size_ || data.size() > size_ - offset)
        {
            return std::unexpected(makeError(ErrorCode::InvalidArgument,
                                             "writeBuffer: the range offset + size is past the end of the buffer"));
        }
        if (!data.empty())
        {
            // The memory is coherent, so no flush is needed.
            std::memcpy(mapped_ + offset, data.data(), data.size());
        }
        return {};
    }
} // namespace ngen::rhi::vulkan
