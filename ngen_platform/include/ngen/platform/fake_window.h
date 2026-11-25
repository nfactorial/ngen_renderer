// Spec: platform. Behaviour changes go through it.
#pragma once

#include <ngen/platform/window.h>

namespace ngen::platform
{
    // A Window whose state and events a test sets directly.
    class FakeWindow final : public Window
    {
    public:
        FakeWindow(NativeWindow handles, PixelSize size);

        // Sets the size and queues a Resized event.
        void resize(PixelSize size);
        // Queues Suspended; nativeWindow() returns SurfaceLost until resume.
        void suspend();
        // Sets the handles and queues Resumed with the current size.
        void resume(NativeWindow handles);
        // Queues CloseRequested.
        void requestClose();

        [[nodiscard]] std::expected<NativeWindow, ngen::Error> nativeWindow() const override;
        [[nodiscard]] PixelSize pixelSize() const override;
        void pollEvents(std::vector<WindowEvent> &events) override;

    private:
        NativeWindow handles_;
        PixelSize size_;
        bool suspended_ = false;
        std::vector<WindowEvent> pending_;
    };
} // namespace ngen::platform
