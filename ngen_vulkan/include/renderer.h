
#ifndef NGEN_VULKAN_RENDERER
#define NGEN_VULKAN_RENDERER

#include "vulkan_context.h"
#include "command_pool.h"
#include "render_pass.h"
#include "semaphore.h"

namespace ngen::vulkan {
    class Semaphore;

    //! \brief It may be that 'Renderer' is too high level of a name for this object. And perhaps this is wrapped into
    //!  the 'Context' object in the future. This would streamline some of the code, as it is turning out to simply
    //!  be a proxy to the context object at the moment.
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer &other) = delete;
        Renderer& operator=(const Renderer &other) = delete;

        void dispose();
        bool initialize(SDL_Window *window, const char *applicationName, bool enableValidation);

        bool beginFrame(const Semaphore &imageAvailable, const CommandPool &commandPool);
        void endFrame(const Semaphore &renderFinished);

        [[nodiscard]] Device& getDevice();
        [[nodiscard]] const Device& getDevice() const;

        [[nodiscard]] VulkanContext& getContext();
        [[nodiscard]] const VulkanContext& getContext() const;

    private:
        uint32_t        m_imageIndex;
        bool            m_initialized;
        VulkanContext   m_context;
    };

    inline Device& Renderer::getDevice() {
        return m_context.getDevice();
    }

    inline const Device& Renderer::getDevice() const {
        return m_context.getDevice();
    }

    inline VulkanContext& Renderer::getContext() {
        return m_context;
    }

    inline const VulkanContext& Renderer::getContext() const {
        return m_context;
    }
}

#endif //NGEN_VULKAN_RENDERER
