
#include "renderer.h"

namespace ngen::vulkan {
    Renderer::~Renderer() {
        dispose();
    }

    //! \brief  Releases all resources currently allocated by the renderer.
    void Renderer::dispose() {
        m_context.dispose();
    }

    //! \brief Prepares the rendering system for use by the application.
    //! \param window [in] - The window the renderer will present its content to.
    //! \param applicationName [in] - The name to be displayed in the application title bar.
    //! \returns <em>True</em> if the renderer initialized successfully otherwise <em>false</em>.
    bool Renderer::initialize(SDL_Window *window, const char *applicationName) {
        if (!m_context.initialize(window, applicationName)) {
            return false;
        }

        return false;
    }
}
