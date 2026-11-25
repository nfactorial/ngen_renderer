// Spec: platform. Behaviour changes go through it.
#pragma once

#include <ngen/error.h>

#include <cstdint>
#include <expected>
#include <variant>
#include <vector>

namespace ngen::platform
{
    // The drawable size in pixels (not points); either may be 0 while minimised.
    struct PixelSize
    {
        uint32_t width;
        uint32_t height;

        friend bool operator==(const PixelSize &, const PixelSize &) = default;
    };

    struct Win32Handles
    {
        void *hwnd;
        void *hinstance;

        friend bool operator==(const Win32Handles &, const Win32Handles &) = default;
    };

    struct AndroidHandles
    {
        void *nativeWindow; // an ANativeWindow*

        friend bool operator==(const AndroidHandles &, const AndroidHandles &) = default;
    };

    struct AppleHandles
    {
        void *metalLayer; // a CAMetalLayer*

        friend bool operator==(const AppleHandles &, const AppleHandles &) = default;
    };

    using NativeWindow = std::variant<Win32Handles, AndroidHandles, AppleHandles>;

    struct WindowEvent
    {
        enum class Kind
        {
            Resized,
            Suspended,
            Resumed,
            CloseRequested,
        };

        Kind kind;
        PixelSize size; // the new size for Resized and Resumed, {0, 0} otherwise

        friend bool operator==(const WindowEvent &, const WindowEvent &) = default;
    };

    class Window
    {
    public:
        virtual ~Window() = default;

        // The current native handles; SurfaceLost while suspended. Handles from before a suspension
        // are invalid afterwards, and those returned after Resumed may differ from them.
        [[nodiscard]] virtual std::expected<NativeWindow, ngen::Error> nativeWindow() const = 0;

        // The current drawable size.
        [[nodiscard]] virtual PixelSize pixelSize() const = 0;

        // Appends every event since the last call, in the order they happened.
        virtual void pollEvents(std::vector<WindowEvent> &events) = 0;
    };
} // namespace ngen::platform
