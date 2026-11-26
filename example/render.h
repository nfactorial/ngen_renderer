// Spec: example. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>
#include <ngen/rhi/command_list.h>
#include <ngen/rhi/device.h>
#include <ngen/rhi/handle.h>

#include <cstddef>
#include <expected>
#include <vector>

namespace example
{
    // The compiled shader, relative to the working directory.
    inline constexpr const char *kTriangleShaderPath = "shaders/triangle.spv";

    // Reads the compiled shader. NotFound, naming the path, when it cannot be opened.
    [[nodiscard]] std::expected<std::vector<std::byte>, ngen::Error> loadShader(const char *path);

    // The triangle's GPU resources. It destroys what it created on destruction, so it must be destroyed
    // before the device it was created with.
    class Triangle
    {
    public:
        explicit Triangle(ngen::rhi::Device &device);
        ~Triangle();
        Triangle(const Triangle &) = delete;
        Triangle &operator=(const Triangle &) = delete;

        // Creates the vertex buffer, shader and pipeline from the compiled shader's bytes.
        [[nodiscard]] std::expected<void, ngen::Error> create(const std::vector<std::byte> &shaderBytes);

        // Clears to black and draws the triangle over the whole frame.
        void draw(ngen::rhi::CommandList &commands) const;

    private:
        ngen::rhi::Device &device_;
        ngen::rhi::BufferHandle vertexBuffer_;
        ngen::rhi::ShaderHandle shader_;
        ngen::rhi::PipelineHandle pipeline_;
    };
} // namespace example
