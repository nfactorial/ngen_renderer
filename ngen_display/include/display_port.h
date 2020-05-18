//
// Represents a display surface within the rendering engine.
//

#ifndef NGEN_DISPLAY_PORT
#define NGEN_DISPLAY_PORT

////////////////////////////////////////////////////////////////////////////

#include <vector>


////////////////////////////////////////////////////////////////////////////

class Ray;

namespace ngen::display {
    class DisplayPort {
    public:
        DisplayPort();
        ~DisplayPort() = default;

        void renderContent();
        void createRay(Ray &ray, float x, float y);

    private:
        std::vector<DisplayPort*> m_children;

        DisplayPort *m_parent;
        bool        m_hidden;
        int         m_x;
        int         m_y;
        int         m_width;
        int         m_height;
    };
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_DISPLAY_PORT
