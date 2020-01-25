project(ngen_render_example)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IoKit REQUIRED)
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(QUARTZ_LIBRARY QuartzCore REQUIRED)
    find_library(IOSURFACE_LIBRARY IoSurface REQUIRED)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation REQUIRED)

    mark_as_advanced(
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${METAL_LIBRARY}
            ${QUARTZ_LIBRARY}
            ${IOSURFACE_LIBRARY}
            ${CORE_FOUNDATION_LIBRARY}
    )

    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
else()
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)
add_definitions(-DSDL_STATIC)

find_package(SDL2 REQUIRED)
find_library(MoltenVK PATHS "$ENV{MOLTEN_VK}/macOS")
MESSAGE("MoltenVK = ${MoltenVK}")
MESSAGE("MOLTEN_VK = ${MOLTEN_VK}")
MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
MESSAGE( "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})

add_executable(example
        main.cpp
        )

target_include_directories(example PUBLIC
        $<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
        )

if(APPLE)
    target_include_directories(example PRIVATE $ENV{MOLTEN_VK}/include)
else()
    target_include_directories(example PRIVATE Vulkan::Vulkan)
endif()

# target_link_libraries(example Vulkan::Vulkan)
target_link_libraries(example SDL2::Main)
target_link_libraries(example ngen::vulkan)

if(APPLE)
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
    target_link_libraries(example Vulkan::Vulkan)
    # target_link_libraries(example ${MoltenVK_LIBRARY})
else()
    target_link_libraries(example Vulkan::Vulkan)
endif()
