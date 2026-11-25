// Spec: rhi. Behaviour changes go through it.
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace ngen::rhi
{
    // Names a resource. The null handle is {0, 0}; issued handles have a generation of 1 or more.
    template <typename Tag>
    struct Handle
    {
        uint32_t index = 0;
        uint32_t generation = 0;

        [[nodiscard]] bool isNull() const
        {
            return generation == 0;
        }

        friend bool operator==(const Handle &, const Handle &) = default;
    };

    using BufferHandle = Handle<struct BufferTag>;
    using ShaderHandle = Handle<struct ShaderTag>;
    using PipelineHandle = Handle<struct PipelineTag>;

    // Issues handles for values and tells a live handle from a stale one.
    template <typename Tag, typename T>
    class HandlePool
    {
    public:
        Handle<Tag> insert(T value)
        {
            uint32_t index;
            if (!freeSlots_.empty())
            {
                index = freeSlots_.back();
                freeSlots_.pop_back();
            }
            else
            {
                index = static_cast<uint32_t>(slots_.size());
                slots_.emplace_back();
            }

            Slot &slot = slots_[index];
            slot.value = std::move(value);
            ++liveCount_;
            return Handle<Tag>{index, slot.generation};
        }

        // The value, or nullptr for a null, stale or out-of-range handle.
        T *get(const Handle<Tag> handle)
        {
            Slot *slot = find(handle);
            return slot != nullptr ? &*slot->value : nullptr;
        }

        const T *get(const Handle<Tag> handle) const
        {
            const Slot *slot = find(handle);
            return slot != nullptr ? &*slot->value : nullptr;
        }

        // Removes and returns the value; empty for a handle get would reject.
        std::optional<T> remove(const Handle<Tag> handle)
        {
            Slot *slot = find(handle);
            if (slot == nullptr)
            {
                return std::nullopt;
            }

            std::optional<T> removed = std::move(slot->value);
            slot->value.reset();
            ++slot->generation;
            if (slot->generation == 0)
            {
                slot->generation = 1;
            }
            freeSlots_.push_back(handle.index);
            --liveCount_;
            return removed;
        }

        // The number of live values.
        [[nodiscard]] size_t size() const
        {
            return liveCount_;
        }

    private:
        struct Slot
        {
            uint32_t generation = 1;
            std::optional<T> value;
        };

        Slot *find(const Handle<Tag> handle)
        {
            return const_cast<Slot *>(std::as_const(*this).find(handle));
        }

        const Slot *find(const Handle<Tag> handle) const
        {
            if (handle.isNull() || handle.index >= slots_.size())
            {
                return nullptr;
            }
            const Slot &slot = slots_[handle.index];
            return slot.value.has_value() && slot.generation == handle.generation ? &slot : nullptr;
        }

        std::vector<Slot> slots_;
        std::vector<uint32_t> freeSlots_;
        size_t liveCount_ = 0;
    };
} // namespace ngen::rhi
