
#include "display_port.h"

namespace ngen {
    namespace display {
        const std::size_t DEFAULT_CHILD_CAPACITY = 32;

        DisplayPort::DisplayPort()
        : m_parent(nullptr)
        , m_hidden(false)
        , m_x(0)
        , m_y(0)
        , m_width(0)
        , m_height(0) {
            m_children.reserve(DEFAULT_CHILD_CAPACITY);
        }

        DisplayPort::~DisplayPort() {
            //
        }

        //! \brief Renders the contents of tbe display port.
        void DisplayPort::renderContent() {

        }

        //! \brief Creates a new ray that passes through the display port at the specified location.
        //! \param ray [in/out] - The Ray instance where the result will be stored.
        //! \param x [in] - Distance along the horizontal axis of the ray to be created.
        //! \param y [in] - Distance along the vertical axis of the ray to be created.
        void DisplayPort::createRay(Ray &ray, float x, float y) {

        }
    }
}
