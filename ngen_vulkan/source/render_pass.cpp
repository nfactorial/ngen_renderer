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

#include "render_pass.h"
#include "device.h"

namespace ngen::vulkan {
    RenderPass::RenderPass()
    : m_handle(nullptr)
    , m_device(nullptr) {
    }

    RenderPass::~RenderPass() {
        dispose();
    }

    //! Destroys any resources allocated by this object.
    void RenderPass::dispose() {
        if (m_handle != nullptr) {
            vkDestroyRenderPass(m_device, m_handle, nullptr);

            m_handle = nullptr;
            m_device = nullptr;
        }
    }

    //! \brief  Prepares the object for use by the application.
    //! \param  device [in] - The device to use when creating the render pass.
    //! \returns <em>True</em> if the render pass was created successfully otherwise <em>false</em>.
    bool RenderPass::create(Device &device) {
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = nullptr;//&colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = nullptr;//&subpass;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS) {
            printf("Failed to create render pass");
            return false;
        }

        return false;
    }
}
