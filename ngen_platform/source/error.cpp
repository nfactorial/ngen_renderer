// Spec: platform. Behaviour changes go through it.
#include <ngen/error.h>

#include <utility>

namespace ngen
{
    namespace
    {
        const char *codeName(const ErrorCode code)
        {
            switch (code)
            {
            case ErrorCode::InvalidArgument: return "InvalidArgument";
            case ErrorCode::Unsupported: return "Unsupported";
            case ErrorCode::OutOfMemory: return "OutOfMemory";
            case ErrorCode::NotFound: return "NotFound";
            case ErrorCode::SurfaceLost: return "SurfaceLost";
            case ErrorCode::DeviceLost: return "DeviceLost";
            case ErrorCode::Internal: return "Internal";
            }
            return "Unknown";
        }
    } // namespace

    Error makeError(const ErrorCode code, std::string message)
    {
        if (message.empty())
        {
            message = std::string("unspecified ") + codeName(code) + " error";
        }
        return Error{code, std::move(message)};
    }
} // namespace ngen
