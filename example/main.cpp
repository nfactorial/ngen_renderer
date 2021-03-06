
#include <cstdio>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_video.h>

#include "render.h"

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

    printf("ngen example\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        printf( "Failed to initialize SDL - Error: %s\n", SDL_GetError());
    } else {
        ngen::vulkan::VulkanContext::dumpExtensions();

        SDL_Window *window = SDL_CreateWindow(
              kApplicationTitle
            , SDL_WINDOWPOS_UNDEFINED
            , SDL_WINDOWPOS_UNDEFINED
            , SCREEN_WIDTH
            , SCREEN_HEIGHT
            , SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN
        );

        if (!window) {
            printf("Failed to create window for testing.\n");
            printf("%s\n", SDL_GetError());
        } else {
            example::Render render;

            render.initialize(window, kApplicationTitle);

            printf("main entry point\n");

            bool running = true;
            while (running) {
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    processEvent(event, running);
                }

                render.render();

                SDL_Delay(1);
            }

            render.dispose();

            SDL_DestroyWindow(window);
        }

        SDL_Quit();
    }

    return result;
}
