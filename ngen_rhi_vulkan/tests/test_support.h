// Spec: rhi-vulkan. Behaviour changes go through it.
#pragma once

#include <ngen/rhi/descriptions.h>
#include <ngen/rhi/device.h>
#include <ngen/rhi/handle.h>

#include <cstddef>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ngen::rhi::vulkan::test_support
{
    struct LoggedMessage
    {
        Severity severity;
        std::string text;
    };

    // Collects what a device sends to its message sink. The sink shares the log, so it may outlive this object.
    class MessageLog
    {
    public:
        [[nodiscard]] MessageSink sink() const
        {
            return [messages = messages_](const Severity severity, const std::string_view text) {
                messages->push_back(LoggedMessage{severity, std::string(text)});
            };
        }

        [[nodiscard]] const std::vector<LoggedMessage> &messages() const
        {
            return *messages_;
        }

        // The Warning and Error messages, one per line, for a failure message; empty when there are none.
        [[nodiscard]] std::string problems() const
        {
            std::string text;
            for (const LoggedMessage &message : *messages_)
            {
                if (message.severity != Severity::Info)
                {
                    text += (message.severity == Severity::Error ? "[error] " : "[warning] ") + message.text + "\n";
                }
            }
            return text;
        }

        void clear() const
        {
            messages_->clear();
        }

    private:
        std::shared_ptr<std::vector<LoggedMessage>> messages_ = std::make_shared<std::vector<LoggedMessage>>();
    };

    // The SPIR-V the build compiles from tests/shaders/test_triangle.slang; empty if the file is missing.
    inline std::vector<std::byte> loadTestShader()
    {
        std::ifstream file(NGEN_TEST_SHADER_PATH, std::ios::binary);
        const std::vector<char> bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        std::vector<std::byte> code(bytes.size());
        for (size_t i = 0; i < bytes.size(); ++i)
        {
            code[i] = static_cast<std::byte>(bytes[i]);
        }
        return code;
    }

    // A pipeline using both of the test shader's entry points, reading float3 positions from binding 0.
    inline GraphicsPipelineDescription testPipelineDescription(const ShaderHandle shader)
    {
        GraphicsPipelineDescription description;
        description.vertex = ShaderEntry{shader, "vertexMain"};
        description.fragment = ShaderEntry{shader, "fragmentMain"};
        description.bindings = {VertexBinding{0, 3 * sizeof(float)}};
        description.attributes = {VertexAttribute{0, 0, Format::R32G32B32Float, 0}};
        description.cull = CullMode::None;
        return description;
    }

    // One triangle: three float3 positions, 36 bytes.
    inline std::vector<std::byte> triangleVertices()
    {
        const float positions[] = {0.0f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f};
        const auto *bytes = reinterpret_cast<const std::byte *>(positions);
        return std::vector<std::byte>(bytes, bytes + sizeof(positions));
    }
} // namespace ngen::rhi::vulkan::test_support
