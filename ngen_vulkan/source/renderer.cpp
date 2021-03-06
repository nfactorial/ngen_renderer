
#include "renderer.h"
#include "semaphore.h"

namespace ngen::vulkan {
    Renderer::Renderer()
    : m_initialized(false) {

    }

    Renderer::~Renderer() {
        dispose();
    }

    //! \brief  Releases all resources currently allocated by the renderer.
    void Renderer::dispose() {
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

        m_initialized = true;
        return true;
    }

    //! \brief Informs the renderer that a frame submission is about to begin.
    //! \param imageAvailable [in] - Semaphore to use when obtaining an image (this may be wrapped away in the future).
    //! \param commandPool [in] - The command pool containing the frame submissions.
    //! \returns <em>True</em> if the frame began successfully otherwise <em>false</em>.
    bool Renderer::beginFrame(const Semaphore &imageAvailable, const Semaphore &renderFinished, const CommandPool &commandPool) {
        if (!m_context.getSwapChain().acquireNextImage(imageAvailable, VK_NULL_HANDLE, &m_imageIndex)) {
            return false;
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailable};
        VkSemaphore signalSemaphores[] = {renderFinished};

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandPool[m_imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_context.getDevice().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            // TODO: Handle error
            printf("vkQueueSubmit failed\n");
        }

        return true;
    }

    //! \brief Completes the current rendered frame and presents it to the user.
    //! \param renderFinished [in] - The semaphore to use when waiting for the swap operation.
    void Renderer::endFrame(const Semaphore &renderFinished) {
        if (m_initialized) {
            VkSemaphore signalSemaphores[] = {renderFinished};

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.pResults = nullptr; // Optional

            VkSwapchainKHR swapChains[] = {m_context.getSwapChain()};

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &m_imageIndex;

            vkQueuePresentKHR(m_context.getDevice().getGraphicsQueue(), &presentInfo);
            vkQueueWaitIdle(m_context.getDevice().getGraphicsQueue());  // TEMP
        }
    }
}
