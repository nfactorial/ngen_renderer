// Spec: renderer. Behaviour changes go through it.
#include <ngen/renderer/configuration.h>

#include <unordered_set>

namespace ngen::renderer
{
    std::vector<StageDescription> defaultStages()
    {
        return {{"Shadow"}, {"Opaque"}, {"Transparent"}, {"PostProcess"}, {"UI"}};
    }

    std::expected<void, ngen::Error> validate(const RendererConfiguration &configuration)
    {
        if (configuration.stages.empty())
        {
            return std::unexpected(makeError(ErrorCode::InvalidArgument, "validate: the renderer has no stages"));
        }

        std::unordered_set<std::string> seen;
        for (size_t i = 0; i < configuration.stages.size(); ++i)
        {
            const std::string &name = configuration.stages[i].name;
            if (name.empty())
            {
                return std::unexpected(
                    makeError(ErrorCode::InvalidArgument, "validate: stage " + std::to_string(i) + " has an empty name"));
            }
            if (!seen.insert(name).second)
            {
                return std::unexpected(
                    makeError(ErrorCode::InvalidArgument, "validate: stage \"" + name + "\" is named more than once"));
            }
        }
        return {};
    }
} // namespace ngen::renderer
