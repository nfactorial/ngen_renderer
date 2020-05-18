#ifndef NGEN_VULKAN_VERTEX_ATTRIBUTE
#define NGEN_VULKAN_VERTEX_ATTRIBUTE

#include <vulkan/vulkan.h>

namespace ngen::vulkan {
    struct AttributeDesc {
        VkVertexInputAttributeDescription description;

        AttributeDesc& binding(uint32_t b) {
            description.binding = b;
            return *this;
        }

        AttributeDesc& location(uint32_t l) {
            description.location = l;
            return *this;
        }

        AttributeDesc& format(VkFormat f) {
            description.format = f;
            return *this;
        }

        AttributeDesc& offset(uint32_t o) {
            description.offset = o;
            return *this;
        }
    };
}

#endif //ndef NGEN_VULKAN_VERTEX_ATTRIBUTE
