
#ifndef NGEN_WINDOW_SURFACE
#define NGEN_WINDOW_SURFACE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"

// TODO:: We do not want to be tied to SDL in the future, here during early development only
struct SDL_Window;

////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    //! \brief Represents the surface of a display window to be used by the renderer.
    class WindowSurface {
    public:
        WindowSurface();
        ~WindowSurface();

        WindowSurface(const WindowSurface &other) = delete;
        WindowSurface& operator=(const WindowSurface &other) = delete;

        void dispose();
        [[nodiscard]] bool initialize(VkInstance instance, SDL_Window *window);

        bool onWindowResized();

        operator VkSurfaceKHR() const; // NOLINT

        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;

    private:

    private:
        VkSurfaceKHR m_surface;
        VkInstance m_instance;
        int m_height;
        int m_width;
    };

    inline WindowSurface::operator VkSurfaceKHR() const {
        return m_surface;
    }

    //! \returns The width (in pixels) of the surface.
    inline int WindowSurface::getWidth() const {
        return m_width;
    }

    //! \returns The height (in pixels) of the surface.
    inline int WindowSurface::getHeight() const {
        return m_height;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_WINDOW_SURFACE
