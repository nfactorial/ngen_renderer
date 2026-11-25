// Spec: platform. Behaviour changes go through it.
#pragma once

#include <ngen/platform/window.h>

#include <SDL3/SDL.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace ngen::platform
{
    struct WindowDescription
    {
        std::string title;
        uint32_t width;
        uint32_t height;
        bool resizable = true;
    };

    // The SDL3 implementation of Window. It owns the SDL window; SDL itself stays initialised by
    // the application.
    class SdlWindow final : public Window
    {
    public:
        // Requires SDL_Init(SDL_INIT_VIDEO) to have succeeded.
        [[nodiscard]] static std::expected<std::unique_ptr<SdlWindow>, ngen::Error>
        create(const WindowDescription &description);

        ~SdlWindow() override;
        SdlWindow(const SdlWindow &) = delete;
        SdlWindow &operator=(const SdlWindow &) = delete;

        // The SDL window, for games that use SDL directly.
        [[nodiscard]] SDL_Window *sdlWindow() const;

        // The application passes every SDL event it polls; those about this window become
        // WindowEvents for pollEvents.
        void handleEvent(const SDL_Event &event);

        [[nodiscard]] std::expected<NativeWindow, ngen::Error> nativeWindow() const override;
        [[nodiscard]] PixelSize pixelSize() const override;
        void pollEvents(std::vector<WindowEvent> &events) override;

    private:
        SdlWindow(SDL_Window *window, SDL_MetalView metalView);

        SDL_Window *window_;
        SDL_MetalView metalView_; // null except on Apple
        bool suspended_ = false;
        std::vector<WindowEvent> pending_;
    };
} // namespace ngen::platform
