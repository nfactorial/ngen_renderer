//
// Copyright 2019 nfactorial
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef NGEN_WINDOW_SURFACE
#define NGEN_WINDOW_SURFACE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

// TODO: Need to add platform specific implementations
namespace ngen::vulkan {
    class WindowSurface {
    public:
        WindowSurface();
        ~WindowSurface();

        void dispose();
        bool initialize(VkInstance instance, PlatformWindow platformWindow, uint32_t width, uint32_t height);

        VkSurfaceKHR getSurface() const;

        operator VkSurfaceKHR() const; // NOLINT

        uint32_t getWidth() const;
        uint32_t getHeight() const;

    private:

    private:
        VkSurfaceKHR m_surface;
        VkInstance m_instance;
        uint32_t m_height;
        uint32_t m_width;
    };

    inline WindowSurface::operator VkSurfaceKHR() const {
        return m_surface;
    }

    inline VkSurfaceKHR WindowSurface::getSurface() const {
        return m_surface;
    }

    inline uint32_t WindowSurface::getWidth() const {
        return m_width;
    }

    inline uint32_t WindowSurface::getHeight() const {
        return m_height;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_WINDOW_SURFACE
