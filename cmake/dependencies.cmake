# Third-party dependencies.
#
# SDL2: an installed SDL2 (found through its CMake config package, e.g. via CMAKE_PREFIX_PATH or SDL2_DIR)
# is used when available; otherwise it is downloaded and built as a static library.

include(FetchContent)

find_package(SDL2 CONFIG QUIET)

if (NOT SDL2_FOUND)
    message(STATUS "SDL2 not found, fetching it from source")

    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-2.30.9
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SDL2)
endif()

# Installed packages usually provide SDL2::SDL2 (shared); a static source build provides SDL2::SDL2-static.
if (TARGET SDL2::SDL2)
    set(NGEN_SDL2_TARGET SDL2::SDL2)
elseif (TARGET SDL2::SDL2-static)
    set(NGEN_SDL2_TARGET SDL2::SDL2-static)
else()
    message(FATAL_ERROR "SDL2 was found but provides neither SDL2::SDL2 nor SDL2::SDL2-static.")
endif()
