project(ngen_render_example)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
else()
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)
add_definitions(-DSDL_STATIC)

find_package(SDL2 REQUIRED)

MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
MESSAGE( "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})

add_executable(example
        main.cpp
        )

target_include_directories(example PUBLIC
        $<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
        )

target_include_directories(example PRIVATE Vulkan::Vulkan)

target_link_libraries(example Vulkan::Vulkan)
target_link_libraries(example SDL2::Main)
target_link_libraries(example ngen::vulkan)

if(APPLE)
    target_link_libraries(example ${COCOA_LIBRARY})
endif()
