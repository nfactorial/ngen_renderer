// Spec: platform. Behaviour changes go through it.
#pragma once

#include <string>

namespace ngen
{
    enum class ErrorCode
    {
        InvalidArgument,
        Unsupported,
        OutOfMemory,
        NotFound,
        SurfaceLost,
        DeviceLost,
        Internal,
    };

    // What failed (code) and a message saying what failed and why. The message is never empty.
    struct Error
    {
        ErrorCode code;
        std::string message;
    };

    // An empty message is replaced by a generic one naming the code, so the message is never empty.
    Error makeError(ErrorCode code, std::string message);
} // namespace ngen
