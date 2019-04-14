
#include <stdio.h>
#include <SDL.h>
#include <SDL_main.h>
#include <vulkan_context.h>
#include <SDL_syswm.h>

namespace {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    const char* kApplicationTitle = "nGen - Vulkan Test";
}

void processEvent(const SDL_Event &event, bool &running) {
    switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

        default:
            break;
    }
}

int main(int argc, char **argv) {
    int result = -1;

    setbuf(stdout, 0);

    printf("ngen example\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        printf( "Failed to initialize SDL - Error: %s\n", SDL_GetError());
    } else {
        SDL_Window *window = SDL_CreateWindow(
                  kApplicationTitle
                , SDL_WINDOWPOS_UNDEFINED
                , SDL_WINDOWPOS_UNDEFINED
                , SCREEN_WIDTH
                , SCREEN_HEIGHT
                , SDL_WINDOW_SHOWN
        );

        if (!window) {
            printf("Failed to create window for testing.\n");
            printf("%s\n", SDL_GetError());
        } else {
            int width, height;

            SDL_GetWindowSize(window, &width, &height);

            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);
            SDL_GetWindowWMInfo(window, &wmInfo);

            ngen::vulkan::VulkanContext vulkan;
#if defined(__APPLE__)
            vulkan.initialize(wmInfo.info.cocoa.window, kApplicationTitle);
#elif defined(_WIN32)
            vulkan.initialize(wmInfo.info.win.window, static_cast<uint32_t>(width), static_cast<uint32_t>(height), kApplicationTitle);
#else
    #error Unknown platform
#endif

            printf("main entry point\n");

            bool running = true;
            while (running) {
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    processEvent(event, running);
                }

                SDL_Delay(1);
            }

            vulkan.dispose();

            SDL_DestroyWindow(window);
        }

        SDL_Quit();
    }

    return result;
}
