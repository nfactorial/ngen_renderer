#ifndef EXAMPLE_RENDER
#define EXAMPLE_RENDER

#include <vulkan_pipeline.h>
#include <command_pool.h>
#include <render_pass.h>
#include <semaphore.h>
#include <renderer.h>
#include <shader.h>

namespace example {
    class Render {
    public:
        Render();
        ~Render();

        bool initialize(SDL_Window *window, const char *applicationName);
        void dispose();

        void render();

    private:
        void recordCommandBuffer(size_t index);

    private:
        std::vector<VkFramebuffer> m_frameBuffers;

        ngen::vulkan::Renderer m_renderer;
        ngen::vulkan::Shader m_vertexShader;
        ngen::vulkan::Shader m_fragmentShader;
        ngen::vulkan::RenderPass m_renderPass;
        ngen::vulkan::Pipeline m_pipeline;

        ngen::vulkan::Semaphore   m_imageAvailable;
        ngen::vulkan::Semaphore   m_renderFinished;
        ngen::vulkan::CommandPool m_commandPool;
    };
}

#endif //EXAMPLE_RENDER
