
#include <cstdio>
#include <fstream>

#include <SDL.h>
#include <SDL_main.h>
#include <vulkan_context.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>
#include <SDL_video.h>

#include <shader.h>

namespace {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    const char* kApplicationTitle = "nGen - Vulkan Test";

    struct FileData {
        size_t length;
        std::unique_ptr<char[]> ptr;
    };

    FileData loadFile(const char *fileName) {
        FileData result = {0, nullptr};

        std::ifstream file(fileName, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            printf("Could not find shader source \"%s\".\n", fileName);
            return result;
        }

        result.length = file.tellg();
        result.ptr = std::make_unique<char[]>(result.length);

        file.seekg(0);
        file.read(result.ptr.get(), result.length);
        file.close();

        return result;
    }
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

    setbuf(stdout, nullptr);

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
            ngen::vulkan::VulkanContext vulkan;
            vulkan.initialize(window, kApplicationTitle);

            printf("Loading shader data\n");

            auto vertexSource = loadFile("./shaders/vert.spv");
            auto fragmentSource = loadFile("./shaders/frag.spv");

            printf("Creating shaders\n");

            ngen::vulkan::Shader vertexShader;
            ngen::vulkan::Shader fragmentShader;

            vertexShader.create(vulkan.getDevice(), vertexSource.ptr.get(), vertexSource.length);
            fragmentShader.create(vulkan.getDevice(), fragmentSource.ptr.get(), fragmentSource.length);

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
