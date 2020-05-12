project(ngen_render_example)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_library(Threads REQUIRED)

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)
add_definitions(-DSDL_STATIC)

find_package(SDL2 REQUIRED)

MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
MESSAGE( "SDL2_INCLUDE_DIRS = " ${SDL2_INCLUDE_DIRS})

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IoKit REQUIRED)
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(QUARTZ_LIBRARY QuartzCore REQUIRED)
    find_library(IOSURFACE_LIBRARY IoSurface REQUIRED)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(MoltenVK PATHS "$ENV{MOLTEN_VK}/macOS")

    mark_as_advanced(
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${METAL_LIBRARY}
            ${QUARTZ_LIBRARY}
            ${IOSURFACE_LIBRARY}
            ${CORE_FOUNDATION_LIBRARY}
    )

    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)

    MESSAGE("MoltenVK = ${MoltenVK}")
    MESSAGE("MOLTEN_VK = ${MOLTEN_VK}")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "/MT /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT        "/MT /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob1 /D NDEBUG")
else()
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

add_executable(example
    main.cpp
)

target_include_directories(example PUBLIC
    $<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>
)

target_link_libraries(example SDL2::Main)
target_link_libraries(example ngen::vulkan)
target_link_libraries(example Vulkan::Vulkan)

if(APPLE)
    target_include_directories(example PRIVATE $ENV{MOLTEN_VK}/include)

    # target_link_libraries(example "/Users/wcoles/Library/Frameworks/vulkansdk-macos-1.1.130.0/MoltenVK/macOS/framework/MoltenVK.framework/MoltenVK")
    target_link_libraries(example "/Users/wcoles/Library/Frameworks/vulkansdk-macos-1.1.130.0/MoltenVK/macOS/static/libMoltenVK.a")
    # target_link_libraries(example "$ENV{MOLTEN_VK}/macOS/Framework/MoltenVK.framework/MoltenVK")
    # target_link_libraries(example "/Users/wcoles/Library/Frameworks/vulkansdk-macos-1.1.101.0/MoltenVK/macOS/static/MoltenVK")
    # target_link_libraries(example "/Users/wcoles/Library/Frameworks/vulkansdk-macos-1.1.101.0/MoltenVK/macOS/static/libMoltenVK")
    target_link_libraries(example ${COCOA_LIBRARY})
    target_link_libraries(example ${IOKIT_LIBRARY})
    target_link_libraries(example ${METAL_LIBRARY})
    target_link_libraries(example ${QUARTZ_LIBRARY})
    target_link_libraries(example ${IOSURFACE_LIBRARY})
    target_link_libraries(example ${CORE_FOUNDATION_LIBRARY})
    # target_link_libraries(example ${MoltenVK_LIBRARY})
else()
    target_include_directories(example PUBLIC
        $<BUILD_INTERFACE:${Vulkan_INCLUDE_DIRS}
    )
    target_link_libraries(example Vulkan::Vulkan)
endif()
