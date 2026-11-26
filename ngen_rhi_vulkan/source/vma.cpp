// Spec: rhi-vulkan. Behaviour changes go through it.
// VMA's implementation, compiled once. Vulkan functions come from the linked loader.
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>
