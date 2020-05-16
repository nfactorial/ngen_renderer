
#include <fstream>
#include "render.h"

namespace {
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

        result.length = static_cast<size_t>(file.tellg());
        result.ptr = std::make_unique<char[]>(result.length);

        file.seekg(0);
        file.read(result.ptr.get(), result.length);
        file.close();

        return result;
    }
}

namespace example {
    Render::Render() {

    }

    Render::~Render() {
        dispose();
    }

    void Render::dispose() {
        m_renderer.getDevice().waitIdle();

        for (auto f : m_frameBuffers) {
            vkDestroyFramebuffer(m_renderer.getDevice(), f, nullptr);
        }
        m_frameBuffers.clear();

        m_imageAvailable.dispose();
        m_renderFinished.dispose();

        m_pipeline.dispose();
        m_renderPass.dispose();
        m_commandPool.dispose();
        m_fragmentShader.dispose();
        m_vertexShader.dispose();
        m_renderer.dispose();
    }

    bool Render::initialize(SDL_Window *window, const char *applicationName) {
        if (!m_renderer.initialize(window, applicationName)) {
            return false;
        }

        printf("Renderer initialized successfully\n");

        printf("Loading shader data\n");

        auto vertexSource = loadFile("./shaders/vert.spv");
        auto fragmentSource = loadFile("./shaders/frag.spv");

        printf("Creating shaders\n");

        // TODO: Still early days for the API, this is a little messy right now
        m_vertexShader.create(m_renderer.getDevice(), vertexSource.ptr.get(), vertexSource.length);
        m_fragmentShader.create(m_renderer.getDevice(), fragmentSource.ptr.get(), fragmentSource.length);

        if (!m_renderPass.create(m_renderer.getContext())) {
            printf("Failed to create render pass\n");

            dispose();
            return false;
        }

        if (!m_pipeline.initialize(m_renderer.getContext(), m_renderPass, m_renderer.getContext().getSwapChain().getExtent(), m_vertexShader, m_fragmentShader)) {
            printf("Failed to create pipeline\n");

            dispose();
            return false;
        }

        // TODO: Can these next two calls be combined into one?
        if (!m_commandPool.create(m_renderer.getContext(), VK_QUEUE_GRAPHICS_BIT)) {
            printf("Failed to create command pool\n");

            dispose();
            return false;
        }

        if (!m_commandPool.allocateCommandBuffers(m_renderer.getContext().getSwapChain().getImageCount())) {
            printf("Failed to allocate command buffers\n");

            dispose();
            return false;
        }

        if (!m_imageAvailable.create(m_renderer.getDevice()) || !m_renderFinished.create(m_renderer.getDevice())) {
            printf("Failed to create semaphores\n");

            dispose();
            return false;
        }

        if (!m_renderer.getContext().getSwapChain().createFrameBuffers(m_renderPass, m_frameBuffers)) {
            printf("Failed to create frame buffers\n");

            dispose();
            return false;
        }

        for (size_t i = 0; i < m_commandPool.size(); ++i) {
            recordCommandBuffer(i);
        }

        return true;
    }

    void Render::render() {
        static bool done = false;
        if (!done) {    // TEMP: Just draw the first frame for now
            m_renderer.beginFrame(m_imageAvailable, m_commandPool);

            m_renderer.endFrame(m_renderFinished);

            done = true;
        }
    }

    void Render::recordCommandBuffer(size_t index) {
        const auto commandBuffer = m_commandPool.begin(index);
        if (commandBuffer != VK_NULL_HANDLE) {
            if (m_renderPass.begin(m_renderer.getContext().getSwapChain().getExtent(), commandBuffer, m_frameBuffers[index])) {
                m_pipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

                vkCmdDraw(commandBuffer, 3, 1, 0, 0);

                m_renderPass.end(commandBuffer);
            }

            m_commandPool.end(index);
        }
    }
}
