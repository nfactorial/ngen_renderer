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

        void dispose(VkInstance instance);
        bool initialize(VkInstance instance, PlatformWindow platformWindow);

        VkSurfaceKHR getSurface() const;

    private:

    private:
        VkSurfaceKHR m_surface;
    };

    inline VkSurfaceKHR WindowSurface::getSurface() const {
        return m_surface;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_WINDOW_SURFACE
