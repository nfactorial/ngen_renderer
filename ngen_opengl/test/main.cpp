
#include "gtest/gtest.h"
#include "GL/glew.h"

#include <SDL.h>
#include <SDL_main.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char **argv) {
    int result = -1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf( "Failed to initialize SDL - Error: %s\n", SDL_GetError());
    } else {
        SDL_Window *window = SDL_CreateWindow(
              "SDL Tutorial"
            , SDL_WINDOWPOS_UNDEFINED
            , SDL_WINDOWPOS_UNDEFINED
            , SCREEN_WIDTH
            , SCREEN_HEIGHT
            , SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL
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

                ::testing::InitGoogleTest(&argc, argv);

                printf("main entry point");

                glewInit();

                result = RUN_ALL_TESTS();

                SDL_GL_DeleteContext(context);
            }

            SDL_DestroyWindow(window);
        }

        SDL_Quit();
    }

    return result;
}
