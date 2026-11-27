// Spec: example. Behaviour changes go through it.
#include "render.h"

#include <ngen/rhi/descriptions.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <span>
#include <string>
#include <utility>

namespace
{
    // Interleaved in binding 0: position at offset 0 (location 0), colour at offset 8 (location 1).
    struct Vertex
    {
        float position[2];
        float color[3];
    };

    static_assert(sizeof(Vertex) == 20);

    constexpr std::array<Vertex, 3> kVertices = {{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    }};
} // namespace

namespace example
{
    std::expected<std::vector<std::byte>, ngen::Error> loadShader(const char *const path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            return std::unexpected(ngen::makeError(ngen::ErrorCode::NotFound,
                                                   std::string("could not open the shader \"") + path + "\""));
        }

        std::vector<std::byte> bytes(static_cast<size_t>(file.tellg()));
        file.seekg(0);
        file.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!file)
        {
            return std::unexpected(ngen::makeError(ngen::ErrorCode::Internal,
                                                   std::string("could not read the shader \"") + path + "\""));
        }
        return bytes;
    }

    Triangle::Triangle(ngen::rhi::Device &device) : device_(device)
    {
    }

    Triangle::~Triangle()
    {
        // Destroying a null handle is ignored, so this also cleans up after a failed create.
        device_.destroy(pipeline_);
        device_.destroy(shader_);
        device_.destroy(vertexBuffer_);
    }

    std::expected<void, ngen::Error> Triangle::create(const std::vector<std::byte> &shaderBytes)
    {
        using namespace ngen::rhi;

        auto buffer = device_.createBuffer({sizeof(kVertices), BufferUsage::Vertex});
        if (!buffer)
        {
            return std::unexpected(std::move(buffer.error()));
        }
        vertexBuffer_ = *buffer;

        if (auto written = device_.writeBuffer(vertexBuffer_, 0, std::as_bytes(std::span(kVertices))); !written)
        {
            return std::unexpected(std::move(written.error()));
        }

        auto shader = device_.createShader({shaderBytes});
        if (!shader)
        {
            return std::unexpected(std::move(shader.error()));
        }
        shader_ = *shader;

        GraphicsPipelineDescription description;
        description.vertex = {shader_, "vertexMain"};
        description.fragment = {shader_, "fragmentMain"};
        description.bindings = {{0, sizeof(Vertex)}};
        description.attributes = {
            {0, 0, Format::R32G32Float, offsetof(Vertex, position)},
            {1, 0, Format::R32G32B32Float, offsetof(Vertex, color)},
        };
        description.cull = CullMode::None;

        auto pipeline = device_.createPipeline(description);
        if (!pipeline)
        {
            return std::unexpected(std::move(pipeline.error()));
        }
        pipeline_ = *pipeline;
        return {};
    }

    void Triangle::draw(ngen::renderer::RenderList &list, const ngen::renderer::StageId stage) const
    {
        ngen::renderer::DrawItem item;
        item.stage = stage;
        item.pipeline = pipeline_;
        item.vertexBuffers = {{0, vertexBuffer_}};
        item.vertexCount = static_cast<uint32_t>(kVertices.size());
        list.add(std::move(item));
    }
} // namespace example
