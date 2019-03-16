
#include "GL/glew.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_main.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

    printf("ngen example");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf( "Failed to initialize SDL - Error: %s\n", SDL_GetError());
    } else {
        SDL_Window *window = SDL_CreateWindow(
                "nGen Renderer - Example"
                , SDL_WINDOWPOS_UNDEFINED
                , SDL_WINDOWPOS_UNDEFINED
                , SCREEN_WIDTH
                , SCREEN_HEIGHT
                , SDL_WINDOW_OPENGL
        );

        if (!window) {
            printf("Failed to create window for testing.");
        } else {
            SDL_GLContext context = SDL_GL_CreateContext(window);
            if (!context) {
                printf("Failed to create opengl context - Error: %s\n", SDL_GetError());
            } else {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

                SDL_GL_SetSwapInterval(1);

                printf("main entry point");

                glewInit();

                bool running = true;
                while (running) {
                    SDL_Event event;

                    while (SDL_PollEvent(&event)) {
                        processEvent(event, running);
                    }

                    SDL_Delay(1);
                }

                SDL_GL_DeleteContext(context);
            }

            SDL_DestroyWindow(window);
        }

        SDL_Quit();
    }

    return result;
}
