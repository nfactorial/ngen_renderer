
#ifndef NGEN_VULKAN_PLATFORM
#define NGEN_VULKAN_PLATFORM

#if defined(__APPLE__)
    #include "platform_apple.h"
#elif defined(_WIN32)
    #include "platform_windows.h"
#elif defined(__linux__)
    #error Linux is not currently supported
#else
    #error Unsupported platform
#endif

#endif //ndef NGEN_VULKAN_PLATFORM
