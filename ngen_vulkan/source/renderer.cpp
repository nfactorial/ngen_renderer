
#include "renderer.h"

namespace ngen::vulkan {
    Renderer::Renderer()
    : m_initialized(false) {

    }

    Renderer::~Renderer() {
        dispose();
    }

    //! \brief  Releases all resources currently allocated by the renderer.
    void Renderer::dispose() {
        m_renderPass.dispose();
        m_imageAvailable.dispose();
        m_renderFinished.dispose();
        m_commandPool.dispose();
        m_context.dispose();

        m_initialized = false;
    }

    //! \brief Prepares the rendering system for use by the application.
    //! \param window [in] - The window the renderer will present its content to.
    //! \param applicationName [in] - The name to be displayed in the application title bar.
    //! \returns <em>True</em> if the renderer initialized successfully otherwise <em>false</em>.
    bool Renderer::initialize(SDL_Window *window, const char *applicationName) {
        if (!m_context.initialize(window, applicationName)) {
            return false;
        }

        if (m_commandPool.create(m_context, VK_QUEUE_GRAPHICS_BIT)) {
            if (m_commandPool.allocateCommandBuffers(m_context.getSwapChain().getImageCount())) {
                if (m_renderPass.create(m_context)) {
                    if (m_imageAvailable.create(m_context.getDevice()) && m_renderFinished.create(m_context.getDevice())) {
                        for (size_t i = 0; i < m_commandPool.size(); ++i) {
                            //recordCommandBuffer(i);
                        }

                        m_initialized = true;
                        return true;
                    }
                }
            }
        }

        dispose();
        return false;
    }

    //! \brief Completes the current rendered frame and presents it to the user.
    void Renderer::present() {
        if (m_initialized) {
            VkSemaphore signalSemaphores[] = {m_renderFinished};

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.pResults = nullptr; // Optional

            vkQueuePresentKHR(m_context.getDevice().getGraphicsQueue(), &presentInfo);
        }
    }

    //! \brief Test function that performs some example rendering operations. This will eventually be removed
    void Renderer::renderTest() {
        uint32_t imageIndex;

        if (m_context.getSwapChain().acquireNextImage(m_imageAvailable, VK_NULL_HANDLE, &imageIndex)) {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = {m_imageAvailable};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            if (vkQueueSubmit(m_context.getDevice().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                // TODO: Handle error
                printf("vkQueueSubmit failed\n");
            }
        }

        present();
    }

    void Renderer::recordCommandBuffer(size_t index) {
        if (m_renderPass.begin(m_context, m_commandPool, index)) {
            m_renderPass.draw(3, 1, 0, 0);

            m_renderPass.end();
        }
    }
}
