// Spec: renderer. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>

#include <array>
#include <expected>
#include <string>
#include <vector>

namespace ngen::renderer
{
    enum class LightingPath
    {
        Forward,
    };

    struct StageDescription
    {
        std::string name;
    };

    // Shadow, Opaque, Transparent, PostProcess and UI, in that order.
    std::vector<StageDescription> defaultStages();

    // The fixed description of a renderer: its lighting path and its stages, in order.
    struct RendererConfiguration
    {
        LightingPath lighting = LightingPath::Forward;
        std::vector<StageDescription> stages = defaultStages();
        std::array<float, 4> clearColor = {0, 0, 0, 1};
    };

    // There is at least one stage; stage names are non-empty and unique. A breach is InvalidArgument naming the
    // stage, or saying there are none.
    [[nodiscard]] std::expected<void, ngen::Error> validate(const RendererConfiguration &configuration);
} // namespace ngen::renderer
