# Third-party dependencies.
#
# SDL3: an installed SDL3 (found through its CMake config package, e.g. via CMAKE_PREFIX_PATH or SDL3_DIR)
# is used when available; otherwise it is downloaded and built as a static library, pinned to the newest
# 3.2 release.

include(FetchContent)

find_package(SDL3 CONFIG QUIET)

if (NOT SDL3_FOUND)
    message(STATUS "SDL3 not found, fetching it from source")

    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.2.30
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SDL3)
endif()

if (TARGET SDL3::SDL3)
    set(NGEN_SDL3_TARGET SDL3::SDL3)
elseif (TARGET SDL3::SDL3-static)
    set(NGEN_SDL3_TARGET SDL3::SDL3-static)
else()
    message(FATAL_ERROR "SDL3 was found but provides neither SDL3::SDL3 nor SDL3::SDL3-static.")
endif()

# VulkanMemoryAllocator (header-only), for the Vulkan backend's memory. Its CMake project provides
# GPUOpen::VulkanMemoryAllocator.
FetchContent_Declare(VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.3.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

# GoogleTest and GoogleMock, for the unit tests.
if (NGEN_BUILD_TESTS)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()
