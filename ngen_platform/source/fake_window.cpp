// Spec: platform. Behaviour changes go through it.
#include <ngen/platform/fake_window.h>

namespace ngen::platform
{
    FakeWindow::FakeWindow(const NativeWindow handles, const PixelSize size) : handles_(handles), size_(size)
    {
    }

    void FakeWindow::resize(const PixelSize size)
    {
        size_ = size;
        pending_.push_back({WindowEvent::Kind::Resized, size});
    }

    void FakeWindow::suspend()
    {
        suspended_ = true;
        pending_.push_back({WindowEvent::Kind::Suspended, {0, 0}});
    }

    void FakeWindow::resume(const NativeWindow handles)
    {
        handles_ = handles;
        suspended_ = false;
        pending_.push_back({WindowEvent::Kind::Resumed, size_});
    }

    void FakeWindow::requestClose()
    {
        pending_.push_back({WindowEvent::Kind::CloseRequested, {0, 0}});
    }

    std::expected<NativeWindow, ngen::Error> FakeWindow::nativeWindow() const
    {
        if (suspended_)
        {
            return std::unexpected(makeError(ErrorCode::SurfaceLost, "the window is suspended"));
        }
        return handles_;
    }

    PixelSize FakeWindow::pixelSize() const
    {
        return size_;
    }

    void FakeWindow::pollEvents(std::vector<WindowEvent> &events)
    {
        events.insert(events.end(), pending_.begin(), pending_.end());
        pending_.clear();
    }
} // namespace ngen::platform
