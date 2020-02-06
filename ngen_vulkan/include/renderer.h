
#ifndef NGEN_VULKAN_RENDER_PASS
#define NGEN_VULKAN_RENDER_PASS

#include "vulkan_context.h"

namespace ngen::vulkan {
    class Renderer {
    public:
        Renderer() = default;
        ~Renderer();

        void dispose();
        bool initialize(SDL_Window *window, const char *applicationName);

        [[nodiscard]] Device& getDevice();
        [[nodiscard]] const Device& getDevice() const;

    private:
        VulkanContext   m_context;
    };

    inline Device& Renderer::getDevice() {
        return m_context.getDevice();
    }

    inline const Device& Renderer::getDevice() const {
        return m_context.getDevice();
    }
}

#endif //NGEN_VULKAN_RENDER_PASS
