// Spec: rhi. Behaviour changes go through it.
#include <ngen/rhi/descriptions.h>

#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace ngen::rhi
{
    namespace
    {
        std::unexpected<ngen::Error> invalid(std::string message)
        {
            return std::unexpected(ngen::makeError(ngen::ErrorCode::InvalidArgument, std::move(message)));
        }

        std::string_view name(const ResourceState state)
        {
            switch (state)
            {
            case ResourceState::Undefined:
                return "Undefined";
            case ResourceState::ColorAttachment:
                return "ColorAttachment";
            case ResourceState::ShaderRead:
                return "ShaderRead";
            case ResourceState::Present:
                return "Present";
            }
            return "an unknown state";
        }
    } // namespace

    std::expected<void, ngen::Error> validate(const BufferDescription &description)
    {
        if (description.size == 0)
        {
            return invalid("BufferDescription.size must be above 0");
        }

        constexpr uint32_t knownUsage = static_cast<uint32_t>(BufferUsage::Vertex | BufferUsage::Uniform);
        const uint32_t usage = static_cast<uint32_t>(description.usage);
        if (usage == 0 || (usage & ~knownUsage) != 0)
        {
            return invalid("BufferDescription.usage must be a non-empty combination of Vertex and Uniform");
        }
        return {};
    }

    std::expected<void, ngen::Error> validate(const ShaderDescription &description)
    {
        if (description.bytecode.empty())
        {
            return invalid("ShaderDescription.bytecode must not be empty");
        }
        return {};
    }

    std::expected<void, ngen::Error> validate(const GraphicsPipelineDescription &description)
    {
        if (description.vertex.entryPoint.empty())
        {
            return invalid("GraphicsPipelineDescription.vertex.entryPoint must not be empty");
        }
        if (description.fragment.entryPoint.empty())
        {
            return invalid("GraphicsPipelineDescription.fragment.entryPoint must not be empty");
        }

        std::set<uint32_t> bindings;
        for (size_t i = 0; i < description.bindings.size(); ++i)
        {
            const uint32_t binding = description.bindings[i].binding;
            if (!bindings.insert(binding).second)
            {
                return invalid("GraphicsPipelineDescription.bindings[" + std::to_string(i) + "].binding " +
                               std::to_string(binding) + " is used more than once");
            }
        }

        std::set<uint32_t> locations;
        for (size_t i = 0; i < description.attributes.size(); ++i)
        {
            const VertexAttribute &attribute = description.attributes[i];
            const std::string prefix = "GraphicsPipelineDescription.attributes[" + std::to_string(i) + "].";
            if (!bindings.contains(attribute.binding))
            {
                return invalid(prefix + "binding " + std::to_string(attribute.binding) +
                               " is not in GraphicsPipelineDescription.bindings");
            }
            if (!locations.insert(attribute.location).second)
            {
                return invalid(prefix + "location " + std::to_string(attribute.location) + " is used more than once");
            }
        }
        return {};
    }

    std::expected<void, ngen::Error> validate(const Barrier &barrier, const ResourceState trackedState)
    {
        // The swap chain image is only ever rendered to and presented.
        const bool shaderReadAllowed = std::visit([](const SwapChainImage &) { return false; }, barrier.target);
        if (!shaderReadAllowed &&
            (barrier.before == ResourceState::ShaderRead || barrier.after == ResourceState::ShaderRead))
        {
            return invalid("barrier: the swap chain image is never read by shaders, so it is never ShaderRead");
        }
        if (barrier.after == ResourceState::Undefined)
        {
            return invalid("barrier: after must not be Undefined");
        }
        if (barrier.before != trackedState)
        {
            return invalid("barrier: before is " + std::string(name(barrier.before)) + ", but the target is " +
                           std::string(name(trackedState)));
        }
        return {};
    }
} // namespace ngen::rhi
