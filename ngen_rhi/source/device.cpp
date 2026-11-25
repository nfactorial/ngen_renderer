// Spec: rhi. Behaviour changes go through it.
#include <ngen/rhi/device.h>

#include <cstdio>

namespace ngen::rhi
{
    MessageSink makeDefaultMessageSink()
    {
        return [](const Severity severity, const std::string_view message) {
            if (severity == Severity::Info)
            {
                return;
            }
            const char *label = severity == Severity::Error ? "error" : "warning";
            std::fprintf(stderr, "ngen [%s] %.*s\n", label, static_cast<int>(message.size()), message.data());
        };
    }
} // namespace ngen::rhi
