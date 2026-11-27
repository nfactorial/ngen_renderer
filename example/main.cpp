// Spec: example. Behaviour changes go through it.
#include "render.h"

#include <ngen/error.h>
#include <ngen/platform/sdl_window.h>
#include <ngen/platform/window.h>
#include <ngen/renderer/render_list.h>
#include <ngen/renderer/renderer.h>
#include <ngen/rhi/device.h>
#include <ngen/rhi/vulkan/vulkan_device.h>

#include <SDL3/SDL.h>

#include <cstdio>
#include <expected>
#include <memory>
#include <utility>
#include <vector>

namespace
{
    constexpr const char *kApplicationTitle = "nGen - Vulkan Test";
    constexpr uint32_t kWindowWidth = 640;
    constexpr uint32_t kWindowHeight = 480;
    constexpr Sint32 kIdleWaitMs = 100;

    // Drains the window's events into the device. Returns false once the window asks to close.
    std::expected<bool, ngen::Error> forwardWindowEvents(ngen::platform::SdlWindow &window, ngen::rhi::Device &device,
                                                         std::vector<ngen::platform::WindowEvent> &events)
    {
        using Kind = ngen::platform::WindowEvent::Kind;

        bool running = true;
        events.clear();
        window.pollEvents(events);
        for (const auto &event : events)
        {
            switch (event.kind)
            {
            case Kind::Resized:
                device.onResized(event.size);
                break;
            case Kind::Suspended:
                device.onSuspended();
                break;
            case Kind::Resumed:
            {
                // Handles from before the suspension are invalid, so fetch fresh ones.
                const auto handles = window.nativeWindow();
                if (!handles)
                {
                    return std::unexpected(handles.error());
                }
                if (auto resumed = device.onResumed(*handles, event.size); !resumed)
                {
                    return std::unexpected(std::move(resumed.error()));
                }
                break;
            }
            case Kind::CloseRequested:
                running = false;
                break;
            }
        }
        return running;
    }

    // Everything lives in this scope, so the triangle and renderer are destroyed before the device and the device
    // before the window; the caller runs SDL_Quit after.
    std::expected<void, ngen::Error> runExample(const std::vector<std::byte> &shaderBytes)
    {
        auto window = ngen::platform::SdlWindow::create({kApplicationTitle, kWindowWidth, kWindowHeight, true});
        if (!window)
        {
            return std::unexpected(std::move(window.error()));
        }

        ngen::rhi::DeviceDescription description;
        description.applicationName = kApplicationTitle;
        description.enableValidation = true;
        auto device = ngen::rhi::vulkan::VulkanDevice::create(description, ngen::rhi::vulkan::WindowSupport::Expected);
        if (!device)
        {
            return std::unexpected(std::move(device.error()));
        }

        const auto handles = (*window)->nativeWindow();
        if (!handles)
        {
            return std::unexpected(handles.error());
        }
        if (auto attached = (*device)->attachWindow(*handles, (*window)->pixelSize()); !attached)
        {
            return std::unexpected(std::move(attached.error()));
        }

        auto renderer = ngen::renderer::Renderer::create(**device, {});
        if (!renderer)
        {
            return std::unexpected(std::move(renderer.error()));
        }
        const auto opaque = (*renderer)->stage("Opaque");
        if (!opaque)
        {
            return std::unexpected(ngen::makeError(ngen::ErrorCode::Internal, "the renderer has no Opaque stage"));
        }

        example::Triangle triangle(**device);
        if (auto created = triangle.create(shaderBytes); !created)
        {
            return std::unexpected(std::move(created.error()));
        }

        std::vector<ngen::platform::WindowEvent> events;
        ngen::renderer::RenderList list;
        bool idle = false; // the frame was skipped last frame, so wait for events instead of spinning
        for (;;)
        {
            SDL_Event event;
            if (idle && SDL_WaitEventTimeout(&event, kIdleWaitMs))
            {
                (*window)->handleEvent(event);
            }
            while (SDL_PollEvent(&event))
            {
                (*window)->handleEvent(event);
            }

            const auto running = forwardWindowEvents(**window, **device, events);
            if (!running)
            {
                return std::unexpected(running.error());
            }
            if (!*running)
            {
                break;
            }

            list.clear();
            triangle.draw(list, *opaque);
            const auto frame = (*renderer)->render(list);
            if (!frame)
            {
                return std::unexpected(frame.error());
            }
            idle = *frame == ngen::renderer::FrameResult::Skipped;
        }

        (*device)->waitIdle();
        return {};
    }
} // namespace

int main()
{
    // The shader is read first, so a missing one is reported before anything is created.
    const auto shaderBytes = example::loadShader(example::kTriangleShaderPath);
    if (!shaderBytes)
    {
        std::fprintf(stderr, "%s\n", shaderBytes.error().message.c_str());
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::fprintf(stderr, "Failed to initialise SDL: %s\n", SDL_GetError());
        return 1;
    }

    const auto result = runExample(*shaderBytes);
    SDL_Quit();

    if (!result)
    {
        std::fprintf(stderr, "%s\n", result.error().message.c_str());
        return 1;
    }
    return 0;
}
