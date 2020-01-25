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

#ifndef NGEN_VULKAN_RENDER_PASS
#define NGEN_VULKAN_RENDER_PASS

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen::vulkan {
    class Device;

    class RenderPass {
    public:
        RenderPass();
        ~RenderPass();

        void dispose();
        bool create(Device &device);

        operator VkRenderPass() const;  // NOLINT

    private:
        VkRenderPass m_handle;
        VkDevice m_device;
    };

    inline RenderPass::operator VkRenderPass() const {
        return m_handle;
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_VULKAN_RENDER_PASS
