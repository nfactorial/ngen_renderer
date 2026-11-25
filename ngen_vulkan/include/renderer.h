
#ifndef NGEN_VULKAN_RENDERER
#define NGEN_VULKAN_RENDERER

#include <array>
#include <memory>
#include <vector>
#include "vulkan_context.h"
#include "command_pool.h"
#include "render_pass.h"
#include "semaphore.h"
#include "fence.h"

namespace ngen::vulkan {
    //! \brief It may be that 'Renderer' is too high level of a name for this object. And perhaps this is wrapped into
    //!  the 'Context' object in the future. This would streamline some of the code, as it is turning out to simply
    //!  be a proxy to the context object at the moment.
    //!
    //! The renderer owns the per-frame synchronisation: up to kMaxFramesInFlight frames may be queued on the GPU
    //! at once, each with its own image-available semaphore and in-flight fence. Render-finished semaphores are
    //! kept per swap chain image, as the presentation engine may still be waiting on one when the frame slot is reused.
    class Renderer {
    public:
        static constexpr uint32_t kMaxFramesInFlight = 2;

        Renderer();
        ~Renderer();

        Renderer(const Renderer &other) = delete;
        Renderer& operator=(const Renderer &other) = delete;

        void dispose();
        bool initialize(SDL_Window *window, const char *applicationName, bool enableValidation);

        bool beginFrame(const CommandPool &commandPool);
        void endFrame();

        [[nodiscard]] Device& getDevice();
        [[nodiscard]] const Device& getDevice() const;

        [[nodiscard]] VulkanContext& getContext();
        [[nodiscard]] const VulkanContext& getContext() const;

    private:
        bool createSyncObjects();
        void destroySyncObjects();

    private:
        uint32_t        m_imageIndex;
        uint32_t        m_frameIndex;
        bool            m_initialized;
        VulkanContext   m_context;

        std::array<Semaphore, kMaxFramesInFlight>   m_imageAvailable;
        std::array<Fence, kMaxFramesInFlight>       m_inFlight;
        std::vector<std::unique_ptr<Semaphore>>     m_renderFinished;   // One per swap chain image.
        std::vector<VkFence>                        m_imagesInFlight;   // Fence of the frame last using each image.
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
