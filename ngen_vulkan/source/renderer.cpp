
#include <cstdio>
#include "renderer.h"
#include "vulkan_error.h"

namespace ngen::vulkan {
    Renderer::Renderer()
    : m_imageIndex(0)
    , m_frameIndex(0)
    , m_initialized(false) {

    }

    Renderer::~Renderer() {
        dispose();
    }

    //! \brief  Releases all resources currently allocated by the renderer.
    void Renderer::dispose() {
        m_context.getDevice().waitIdle();

        destroySyncObjects();
        m_context.dispose();

        m_initialized = false;
    }

    //! \brief Prepares the rendering system for use by the application.
    //! \param window [in] - The window the renderer will present its content to.
    //! \param applicationName [in] - The name to be displayed in the application title bar.
    //! \param enableValidation [in] - If <em>True</em> vulkan will be created with validation layers.
    //! \returns <em>True</em> if the renderer initialized successfully otherwise <em>false</em>.
    bool Renderer::initialize(SDL_Window *window, const char *applicationName, bool enableValidation) {
        if (!m_context.initialize(window, applicationName, enableValidation)) {
            return false;
        }

        if (!createSyncObjects()) {
            printf("Failed to create frame synchronisation objects\n");
            dispose();
            return false;
        }

        m_initialized = true;
        return true;
    }

    //! \brief Creates the semaphores and fences used to pace frame submission.
    //! \returns <em>True</em> if all objects were created successfully otherwise <em>false</em>.
    bool Renderer::createSyncObjects() {
        auto &device = m_context.getDevice();

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
            // Fences start signaled so the first wait on each frame slot returns immediately.
            if (!m_imageAvailable[i].create(device) || !m_inFlight[i].create(device, true)) {
                return false;
            }
        }

        const size_t imageCount = m_context.getSwapChain().getImageCount();

        m_renderFinished.clear();
        for (size_t i = 0; i < imageCount; ++i) {
            m_renderFinished.push_back(std::make_unique<Semaphore>());
            if (!m_renderFinished.back()->create(device)) {
                return false;
            }
        }

        m_imagesInFlight.assign(imageCount, VK_NULL_HANDLE);
        m_frameIndex = 0;

        return true;
    }

    //! \brief Destroys the frame synchronisation objects, the device must be idle.
    void Renderer::destroySyncObjects() {
        m_renderFinished.clear();
        m_imagesInFlight.clear();

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
            m_imageAvailable[i].dispose();
            m_inFlight[i].dispose();
        }
    }

    //! \brief Acquires the next swap chain image and submits its command buffer for rendering.
    //! \param commandPool [in] - The command pool containing one pre-recorded command buffer per swap chain image.
    //! \returns <em>True</em> if the frame was submitted and should be presented with endFrame() otherwise <em>false</em>.
    bool Renderer::beginFrame(const CommandPool &commandPool) {
        if (!m_initialized) {
            return false;
        }

        auto &frameFence = m_inFlight[m_frameIndex];
        frameFence.wait();

        if (!m_context.getSwapChain().acquireNextImage(m_imageAvailable[m_frameIndex], VK_NULL_HANDLE, &m_imageIndex)) {
            return false;
        }

        // The command buffers are per image, so a previous frame may still be executing this image's buffer.
        if (m_imagesInFlight[m_imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(getDevice(), 1, &m_imagesInFlight[m_imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_imagesInFlight[m_imageIndex] = frameFence;

        VkSemaphore waitSemaphores[] = {m_imageAvailable[m_frameIndex]};
        VkSemaphore signalSemaphores[] = {*m_renderFinished[m_imageIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandPool[m_imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // Only reset once we know work will be submitted, otherwise the next wait on this fence would never return.
        frameFence.reset();

        const VkResult result = vkQueueSubmit(getDevice().getGraphicsQueue(), 1, &submitInfo, frameFence);
        if (result != VK_SUCCESS) {
            printf("vkQueueSubmit failed: %s\n", getResultString(result));

            // Leave the frame slot usable: replace the unsignaled fence with a signaled one.
            m_imagesInFlight[m_imageIndex] = VK_NULL_HANDLE;
            frameFence.dispose();
            if (!frameFence.create(getDevice(), true)) {
                m_initialized = false;
            }

            return false;
        }

        return true;
    }

    //! \brief Presents the frame submitted by beginFrame() and advances to the next frame slot.
    void Renderer::endFrame() {
        if (!m_initialized) {
            return;
        }

        VkSemaphore waitSemaphores[] = {*m_renderFinished[m_imageIndex]};
        VkSwapchainKHR swapChains[] = {m_context.getSwapChain()};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_imageIndex;

        // TODO: VK_ERROR_OUT_OF_DATE_KHR / VK_SUBOPTIMAL_KHR should trigger swap chain recreation (resize support).
        vkQueuePresentKHR(getDevice().getPresentationQueue(), &presentInfo);

        m_frameIndex = (m_frameIndex + 1) % kMaxFramesInFlight;
    }
}
