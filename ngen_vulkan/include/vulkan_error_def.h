
#if !defined(VULKAN_ERROR)
    #error VULKAN_ERROR must be defined when including vulkan_error_def.h
#endif //!defined(VULKAN_ERROR)

VULKAN_ERROR(VK_SUCCESS, "Success");
VULKAN_ERROR(VK_NOT_READY, "A fence or query has not yet completed");
VULKAN_ERROR(VK_TIMEOUT, "A wait operation has not completed in the specified time");
VULKAN_ERROR(VK_EVENT_SET, "An event is signaled");
VULKAN_ERROR(VK_EVENT_RESET, "An event is unsignaled");
VULKAN_ERROR(VK_INCOMPLETE, "A return array was too small for the result");
VULKAN_ERROR(VK_ERROR_OUT_OF_HOST_MEMORY, "A host memory allocation has failed");
VULKAN_ERROR(VK_ERROR_OUT_OF_DEVICE_MEMORY, "A device memory allocation has failed");
VULKAN_ERROR(VK_ERROR_INITIALIZATION_FAILED, "Initialization of an object could not be completed for implementation-specific reasons");
VULKAN_ERROR(VK_ERROR_DEVICE_LOST, "The logical or physical device has been lost");
VULKAN_ERROR(VK_ERROR_MEMORY_MAP_FAILED, "Mapping of a memory object has failed");
VULKAN_ERROR(VK_ERROR_LAYER_NOT_PRESENT, "A requested layer is not present or could not be loaded");
VULKAN_ERROR(VK_ERROR_EXTENSION_NOT_PRESENT, "A requested extension is not supported");
VULKAN_ERROR(VK_ERROR_FEATURE_NOT_PRESENT, "A requested feature is not supported");
VULKAN_ERROR(VK_ERROR_INCOMPATIBLE_DRIVER, "The requested version of Vulkan is not supported by the driver or is otherwise incompatible");
VULKAN_ERROR(VK_ERROR_TOO_MANY_OBJECTS, "Too many objects of the type have already been created");
VULKAN_ERROR(VK_ERROR_FORMAT_NOT_SUPPORTED, "A requested format is not supported on this device");
VULKAN_ERROR(VK_ERROR_SURFACE_LOST_KHR, "A surface is no longer available");
VULKAN_ERROR(VK_SUBOPTIMAL_KHR, "A swapchain no longer matches the surface properties exactly, but can still be used");
VULKAN_ERROR(VK_ERROR_OUT_OF_DATE_KHR, "A surface has changed in such a way that it is no longer compatible with the swapchain");
VULKAN_ERROR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "The display used by a swapchain does not use the same presentable image layout");
VULKAN_ERROR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API");
VULKAN_ERROR(VK_ERROR_VALIDATION_FAILED_EXT, "A validation layer found an error");
VULKAN_ERROR(VK_ERROR_FRAGMENTED_POOL, "A pool allocation has failed due to fragmentation of the pool’s memory");
VULKAN_ERROR(VK_ERROR_OUT_OF_POOL_MEMORY, "A pool memory allocation has failed");
VULKAN_ERROR(VK_ERROR_INVALID_EXTERNAL_HANDLE, "An external handle is not a valid handle of the specified type");
VULKAN_ERROR(VK_ERROR_INVALID_SHADER_NV, "One or more shaders failed to compile or link");
VULKAN_ERROR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT");
VULKAN_ERROR(VK_ERROR_FRAGMENTATION_EXT, "A descriptor pool creation has failed due to fragmentation");
VULKAN_ERROR(VK_ERROR_NOT_PERMITTED_EXT, "A requested extension is not permitted in this context");
VULKAN_ERROR(VK_ERROR_INVALID_DEVICE_ADDRESS_EXT, "A buffer creation failed because the requested address is not available");
VULKAN_ERROR(VK_RESULT_RANGE_SIZE, "VK_RESULT_RANGE_SIZE");
VULKAN_ERROR(VK_RESULT_MAX_ENUM, "VK_RESULT_MAX_ENUM");

#undef VULKAN_ERROR
