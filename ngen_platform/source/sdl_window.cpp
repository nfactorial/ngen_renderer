// Spec: platform. Behaviour changes go through it.
#include <ngen/platform/sdl_window.h>

#include <string>

namespace ngen::platform
{
    namespace
    {
        std::string sdlFailure(const char *const what)
        {
            return std::string(what) + ": " + SDL_GetError();
        }
    } // namespace

    std::expected<std::unique_ptr<SdlWindow>, ngen::Error> SdlWindow::create(const WindowDescription &description)
    {
        if (description.title.empty())
        {
            return std::unexpected(makeError(ErrorCode::InvalidArgument, "the window title is empty"));
        }
        if (description.width == 0 || description.height == 0)
        {
            return std::unexpected(makeError(ErrorCode::InvalidArgument, "the window width and height must be non-zero"));
        }

        SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(__APPLE__)
        flags |= SDL_WINDOW_METAL;
#endif
        if (description.resizable)
        {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        SDL_Window *window = SDL_CreateWindow(description.title.c_str(), static_cast<int>(description.width),
                                              static_cast<int>(description.height), flags);
        if (window == nullptr)
        {
            return std::unexpected(makeError(ErrorCode::Internal, sdlFailure("SDL_CreateWindow failed")));
        }

        SDL_MetalView metalView = nullptr;
#if defined(__APPLE__)
        metalView = SDL_Metal_CreateView(window);
        if (metalView == nullptr)
        {
            const std::string message = sdlFailure("SDL_Metal_CreateView failed");
            SDL_DestroyWindow(window);
            return std::unexpected(makeError(ErrorCode::Internal, message));
        }
#endif
        return std::unique_ptr<SdlWindow>(new SdlWindow(window, metalView));
    }

    SdlWindow::SdlWindow(SDL_Window *const window, const SDL_MetalView metalView) : window_(window), metalView_(metalView)
    {
    }

    SdlWindow::~SdlWindow()
    {
#if defined(__APPLE__)
        if (metalView_ != nullptr)
        {
            SDL_Metal_DestroyView(metalView_);
        }
#endif
        SDL_DestroyWindow(window_);
    }

    SDL_Window *SdlWindow::sdlWindow() const
    {
        return window_;
    }

    void SdlWindow::handleEvent(const SDL_Event &event)
    {
        switch (event.type)
        {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            if (event.window.windowID == SDL_GetWindowID(window_))
            {
                pending_.push_back({WindowEvent::Kind::Resized,
                                    {static_cast<uint32_t>(event.window.data1), static_cast<uint32_t>(event.window.data2)}});
            }
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            // Some backends (Windows) send no size change on minimise, so Resized to 0 is made here.
            if (event.window.windowID == SDL_GetWindowID(window_))
            {
                pending_.push_back({WindowEvent::Kind::Resized, {0, 0}});
            }
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            if (event.window.windowID == SDL_GetWindowID(window_))
            {
                pending_.push_back({WindowEvent::Kind::Resized, pixelSize()});
            }
            break;
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
            suspended_ = true;
            pending_.push_back({WindowEvent::Kind::Suspended, {0, 0}});
            break;
        case SDL_EVENT_DID_ENTER_FOREGROUND:
            suspended_ = false;
            pending_.push_back({WindowEvent::Kind::Resumed, pixelSize()});
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (event.window.windowID == SDL_GetWindowID(window_))
            {
                pending_.push_back({WindowEvent::Kind::CloseRequested, {0, 0}});
            }
            break;
        case SDL_EVENT_QUIT:
            pending_.push_back({WindowEvent::Kind::CloseRequested, {0, 0}});
            break;
        default:
            break;
        }
    }

    std::expected<NativeWindow, ngen::Error> SdlWindow::nativeWindow() const
    {
        if (suspended_)
        {
            return std::unexpected(makeError(ErrorCode::SurfaceLost, "the window is suspended"));
        }

#if defined(_WIN32)
        const SDL_PropertiesID properties = SDL_GetWindowProperties(window_);
        void *hwnd = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        void *hinstance = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
        if (hwnd == nullptr)
        {
            return std::unexpected(makeError(ErrorCode::Internal, "SDL reports no Win32 window handle for this window"));
        }
        return NativeWindow{Win32Handles{hwnd, hinstance}};
#elif defined(__ANDROID__)
        const SDL_PropertiesID properties = SDL_GetWindowProperties(window_);
        void *nativeWindow = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
        if (nativeWindow == nullptr)
        {
            return std::unexpected(makeError(ErrorCode::SurfaceLost, "SDL reports no Android native window"));
        }
        return NativeWindow{AndroidHandles{nativeWindow}};
#elif defined(__APPLE__)
        void *layer = SDL_Metal_GetLayer(metalView_);
        if (layer == nullptr)
        {
            return std::unexpected(makeError(ErrorCode::Internal, sdlFailure("SDL_Metal_GetLayer failed")));
        }
        return NativeWindow{AppleHandles{layer}};
#else
        return std::unexpected(makeError(ErrorCode::Unsupported, "SdlWindow has no native handles on this platform"));
#endif
    }

    PixelSize SdlWindow::pixelSize() const
    {
        if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)
        {
            return {0, 0};
        }
        int width = 0;
        int height = 0;
        SDL_GetWindowSizeInPixels(window_, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    void SdlWindow::pollEvents(std::vector<WindowEvent> &events)
    {
        events.insert(events.end(), pending_.begin(), pending_.end());
        pending_.clear();
    }
} // namespace ngen::platform
