
#ifndef NGEN_VULKAN_RENDERER
#define NGEN_VULKAN_RENDERER

#include "vulkan_context.h"
#include "command_pool.h"
#include "render_pass.h"
#include "semaphore.h"

namespace ngen::vulkan {
    class Renderer {
    public:
        Renderer() = default;
        ~Renderer();

        Renderer(const Renderer &other) = delete;
        Renderer& operator=(const Renderer &other) = delete;

        void dispose();
        bool initialize(SDL_Window *window, const char *applicationName);

        void present();

        void renderTest();

        [[nodiscard]] Device& getDevice();
        [[nodiscard]] const Device& getDevice() const;

    private:
        void recordCommandBuffer(size_t index);

    private:
        VulkanContext   m_context;
        CommandPool     m_commandPool;

        // The renderer should not be in control of these next items, they are here for testing purposes.
        Semaphore       m_imageAvailable;
        Semaphore       m_renderFinished;
        RenderPass      m_renderPass;
    };

    inline Device& Renderer::getDevice() {
        return m_context.getDevice();
    }

    inline const Device& Renderer::getDevice() const {
        return m_context.getDevice();
    }
}

#endif //NGEN_VULKAN_RENDERER
