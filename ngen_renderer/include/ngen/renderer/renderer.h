// Spec: renderer. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/renderer/configuration.h>
#include <ngen/renderer/render_list.h>
#include <ngen/rhi/device.h>

#include <expected>
#include <memory>
#include <optional>
#include <string_view>

namespace ngen::renderer
{
    enum class FrameResult
    {
        Presented,
        Skipped,
    };

    // Turns a render list into a frame on the device: one render graph pass per stage that has items, in stage
    // order. Uses the device it was created with, does not own it, and must be destroyed before it.
    class Renderer
    {
    public:
        // Validates the configuration first.
        [[nodiscard]] static std::expected<std::unique_ptr<Renderer>, ngen::Error> create(
            rhi::Device &device, RendererConfiguration configuration);

        // The stage of that name.
        [[nodiscard]] std::optional<StageId> stage(std::string_view name) const;

        [[nodiscard]] std::expected<FrameResult, ngen::Error> render(const RenderList &list);

        [[nodiscard]] const RendererConfiguration &configuration() const;

    private:
        Renderer(rhi::Device &device, RendererConfiguration configuration);

        rhi::Device &device_;
        const RendererConfiguration configuration_;
    };
} // namespace ngen::renderer
